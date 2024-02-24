#include "EMBYAggregationSearch.hpp"

EMBYAggregationSearch::EMBYAggregationSearch(QWidget *parent)
    : QMainWindow(parent)
{

    ui.setupUi(this);
    naManager = new QNetworkAccessManager(this);
    int width = ui.SearchResult->width();
    ui.SearchResult->setColumnWidth(0, width / 2.5);
    ui.SearchResult->setColumnWidth(1, width / 3);
    ui.SearchResult->setColumnWidth(2, width / 10);
    ui.SearchResult->setColumnWidth(3, width / 15);
    connect(ui.GlobalSearch, SIGNAL(clicked()), this, SLOT(onGlobalSearch()));
    connect(ui.AddServer, SIGNAL(clicked()), this, SLOT(onAddServer()));

    connect(ui.Login, SIGNAL(clicked()), this, SLOT(onLogin()));
    ReadConfig();

}

EMBYAggregationSearch::~EMBYAggregationSearch()
{}
