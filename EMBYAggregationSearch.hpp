#pragma once

#include <QtWidgets/QMainWindow>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include < QJsonObject >
#include < QJsonArray >
#include < QJsonDocument >
#include < QJsonValue >
#include < QJsonParseError >
#include <QDesktopServices>
#include <QUrl>
#include <Qlist>
#include <vector>
#include<QMessageBox>
#include<QFile>
#include<QTextStream >
#include<QUrlQuery>
#include "ui_EMBYAggregationSearch.h"


class embyserver
{
public:
	QString Server_url;
	QString Result;
	QString Token;
	embyserver(QString serverurl, QString token)
	{
		Server_url = serverurl;
		Token = token;
	}
	embyserver(QString serverurl, QString Username, QString Password)
	{

	}
};

class GOButton : public QPushButton
{
	Q_OBJECT
public:
	QString url;
	explicit GOButton(QString _url) :QPushButton()
	{
		url = _url;
	}
};

class EMBYAggregationSearch : public QMainWindow
{
    Q_OBJECT

public:
    EMBYAggregationSearch(QWidget *parent = nullptr);
    ~EMBYAggregationSearch();
	QNetworkAccessManager* naManager;
	static QString ConfigFile;
private:
    Ui::EMBYAggregationSearchClass ui;
	std::vector<embyserver *> servers;

	void ReadConfig()
	{
		QFile file(ConfigFile);
		if (!file.open(QFile::ReadOnly | QFile::Text)) {
			QMessageBox::warning(NULL, "WARNING", "Failed to Read "+ConfigFile, QMessageBox::Yes, QMessageBox::Yes);
			abort();
		}

		QByteArray data = file.readAll();
		QJsonParseError err;
		QJsonDocument json_doc = QJsonDocument::fromJson(data, &err);
		if (json_doc.isNull())
		{
			QMessageBox::warning(NULL, "WARNING", "Failed to Parse " + ConfigFile, QMessageBox::Yes, QMessageBox::Yes);
		}
		QJsonObject rootObj = json_doc.object();
		int counter = 0;
		for (auto it = rootObj.begin(); it != rootObj.end(); ++it) {
			embyserver* server = new embyserver(it.key(), it.value().toString());
			servers.push_back(server);
			ui.SearchSelection->addItem(it.key());
			counter++;
		}
		QMessageBox::information(NULL, "Setup", "Launched With "+QString::number(counter) + " Server Configs!", QMessageBox::Yes, QMessageBox::Yes);
	}
private slots:
    void onGlobalSearch() 
	{
		for (int row = ui.SearchResult->rowCount() - 1; row >= 0; row--)
		{
			ui.SearchResult->removeRow(row);
		}
		if (ui.SearchText->text().isEmpty())
			return;
		for (int i = 0; i < servers.size(); i++)
		{
			embyserver* server = servers[i];
			QNetworkRequest request;
			//connect(naManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(requestFinished(QNetworkReply*)));
			request.setUrl(QUrl(server->Server_url + R"(/emby/Items?Fields=BasicSyncInfo&Recursive=true&SearchTerm=)" + ui.SearchText->text() + R"(&X-Emby-Token=)"+server->Token + R"(&X-Emby-Language=zh-cn)"));
			QNetworkReply* reply = naManager->get(request);
			connect(reply, &QNetworkReply::finished, this, [=]() {
				requestFinished(reply, server);
			});
		}
    }
	void requestFinished(QNetworkReply* reply,embyserver* server) 
	{
		QString ERRORmsg;
		// 获取http状态码
		QVariant statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
		if (statusCode.isValid())
			ERRORmsg += QString("status code=%1").arg(statusCode.toInt());

		QVariant reason = reply->attribute(QNetworkRequest::HttpReasonPhraseAttribute).toString();
		if (reason.isValid())
			ERRORmsg += QString("reason=%1").arg(reason.toString());

		QNetworkReply::NetworkError err = reply->error();
		if (err != QNetworkReply::NoError) 
		{
			ERRORmsg += QString("Failed: %1").arg(reply->errorString());
			QMessageBox::warning(NULL, "WARNING", ERRORmsg, QMessageBox::Yes, QMessageBox::Yes);
		}
		else 
		{
			// 获取返回内容
			//Result += reply->readAll();
			// QJsonParseError类用于在JSON解析期间报告错误。
			QJsonParseError jsonError;
			// 将json解析为UTF-8编码的json文档，并从中创建一个QJsonDocument。
			// 如果解析成功，返回QJsonDocument对象，否则返回null
			QJsonDocument result_doc = QJsonDocument::fromJson(reply->readAll(), &jsonError);
			// 判断是否解析失败
			if (jsonError.error != QJsonParseError::NoError && !result_doc.isNull()) {
				qDebug() << "Json格式错误！" << jsonError.error;
				return;
			}
			QJsonObject result_json = result_doc.object();
			QJsonArray items = result_json.value("Items").toArray();
			int index = 0;
			for (int i = 0; i < items.size(); i++) 
			{
				QJsonObject item = items.at(i).toObject();
				if (ui.FliterSelection->currentText()!="ALL"&&(item.value("Type").toString() == "Episode" || item.value("Type").toString() != ui.FliterSelection->currentText()))
					continue;
				ui.SearchResult->insertRow(index);
				ui.SearchResult->setItem(index, 0, new QTableWidgetItem(item.value("Name").toString()));
				ui.SearchResult->setItem(index, 1, new QTableWidgetItem(server->Server_url));
				ui.SearchResult->setItem(index, 2, new QTableWidgetItem(item.value("Type").toString()));
				QString URL = server->Server_url + R"(/web/index.html#!/item?id=)" + item.value("Id").toString() + "&serverId=" + item.value("ServerId").toString();
				GOButton* btn = new GOButton(URL);
				btn->setText("GO!");
				connect(btn, SIGNAL(clicked()), this, SLOT(onGOButton()));
				ui.SearchResult->setCellWidget(index, 3, btn);
				index++;
			}
			/*if (index)
				QMessageBox::information(NULL, "SearchResult", QString::number(index)+" Found!", QMessageBox::Yes, QMessageBox::Yes);
			else
				QMessageBox::warning(NULL, "SearchResult", "No Result!", QMessageBox::Yes, QMessageBox::Yes);*/
		}
	}
	void onGOButton()
	{
		GOButton* button = (GOButton * )qobject_cast<QPushButton*>(sender());	//神奇の获取到触发信号的对象
		QDesktopServices::openUrl(button->url);
	}

	void onAddServer()
	{
		QString serverurl = ui.ServerURL->text();
		QString token = ui.Usertoken->text();
		if (serverurl.isEmpty() || token.isEmpty())
			return;
		embyserver* server = new embyserver(serverurl, token);
		servers.push_back(server);
		ui.SearchSelection->addItem(serverurl);

		// Open the config file in read-write mode
		QFile file(ConfigFile);
		if (!file.open(QFile::ReadWrite | QFile::Text)) {
			QMessageBox::warning(NULL, "WARNING", "Failed to Open " + ConfigFile, QMessageBox::Yes, QMessageBox::Yes);
			return;
		}

		// Read the existing data from the file
		QByteArray data = file.readAll();
		QJsonParseError err;
		QJsonDocument json_doc = QJsonDocument::fromJson(data, &err);
		if (json_doc.isNull())
		{
			QMessageBox::warning(NULL, "WARNING", "Failed to Parse " + ConfigFile, QMessageBox::Yes, QMessageBox::Yes);
			return;
		}

		// Get the root object and insert the new server
		QJsonObject rootObj = json_doc.object();
		rootObj.insert(serverurl, token);

		// Clear the file and write the updated JSON back to the file
		file.resize(0);
		json_doc.setObject(rootObj);
		file.write(json_doc.toJson());
		file.close();
	}

	void onLogin()
	{
		QString URL = ui.Login_ServerURL->text();
		QString Username = ui.Login_Username->text();
		QString Password = ui.Login_Password->text();
		if (URL.isEmpty() || Username.isEmpty())
			return;
		QUrl url(URL+"/emby/Users/authenticatebyname?X-Emby-Client=Emby+Web&X-Emby-Device-Id=dvid&X-Emby-Client-Version=4.8.1.0&X-Emby-Language=zh-cn");
		QNetworkRequest request(url);
		QNetworkAccessManager* lgmanager = new QNetworkAccessManager(this);
		request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded; charset=UTF-8");

		QUrlQuery params;
		params.addQueryItem("Username", Username);
		params.addQueryItem("Pw", Password);  // 这里是负载

		QNetworkReply* reply = lgmanager->post(request, params.query().toUtf8());
		connect(reply, &QNetworkReply::finished, this, [=]() {
			loginrequestfinished(reply);
			});
	}

	void loginrequestfinished(QNetworkReply* reply)
	{
		QString ERRORmsg;
		// 获取http状态码
		QVariant statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
		if (statusCode.isValid())
			ERRORmsg += QString("status code=%1").arg(statusCode.toInt());

		QVariant reason = reply->attribute(QNetworkRequest::HttpReasonPhraseAttribute).toString();
		if (reason.isValid())
			ERRORmsg += QString("reason=%1").arg(reason.toString());

		QNetworkReply::NetworkError err = reply->error();
		if (err != QNetworkReply::NoError)
		{
			ERRORmsg += QString("Failed: %1").arg(reply->errorString());
			QMessageBox::warning(NULL, "WARNING", ERRORmsg, QMessageBox::Yes, QMessageBox::Yes);
		}
		else
		{
			// 获取返回内容
			//Result += reply->readAll();
			// QJsonParseError类用于在JSON解析期间报告错误。
			QJsonParseError jsonError;
			// 将json解析为UTF-8编码的json文档，并从中创建一个QJsonDocument。
			// 如果解析成功，返回QJsonDocument对象，否则返回null
			QJsonDocument result_doc = QJsonDocument::fromJson(reply->readAll(), &jsonError);
			// 判断是否解析失败
			if (jsonError.error != QJsonParseError::NoError && !result_doc.isNull()) {
				qDebug() << "Json格式错误！" << jsonError.error;
				return;
			}
			QJsonObject result_json = result_doc.object();
			QString AccessToken = result_json.value("AccessToken").toString();
			ui.Login_Token->setText(AccessToken);
		}
	}

};


