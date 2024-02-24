#include "EMBYAggregationSearch.hpp"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    EMBYAggregationSearch w;
    EMBYAggregationSearch::ConfigFile = "config.json";
    w.show();
    return a.exec();
}

QString  EMBYAggregationSearch::ConfigFile = "config.json";