#ifndef CHARTWIDGET_H
#define CHARTWIDGET_H

#include <QWidget>
#include <include/externals/nlohmann/json/json.hpp>
#include <include/externals/qcustomplot/qcustomplot.h>
#include "src/terconData.h"

using json = nlohmann::json;

class AdvancedQCustomPlot : public QCustomPlot
{
public:
    AdvancedQCustomPlot();
    bool isZoomed();

protected:
    void wheelEvent ( QWheelEvent * event );
    void mousePressEvent(QMouseEvent *event);

private:
    bool isZoomed_;
};

class GraphData2{
public:
    bool yAxis2;
    QVector <double> x;
    QVector <double> y;
    QString sourceId;
    int multiplier;
};

class ChartWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ChartWidget(QWidget *parent = 0);
    ~ChartWidget();

signals:

public slots:
    void setPlotTitle(const QString & text);
    void addSignalFromJSON (const json & graphSetting);
    void addAxesFromJSON (const json & axesSetting);
    void addDataTercon(TerconData terconData);

private slots:
    /*void contextMenuRequest(QPoint pos);
    void moveLegend();*/

private:
    void initGraph();
    AdvancedQCustomPlot * plot;
    QVector <GraphData2> graphsData;
    bool xAxesClicked;
    bool yAxesClicked;

};

#endif // CHARTWIDGET_H
