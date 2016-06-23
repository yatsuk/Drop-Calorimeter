#ifndef CHARTWIDGET_H
#define CHARTWIDGET_H

#include <QWidget>
#include <QJsonObject>
#include "qcustomplot.h"
#include "terconData.h"

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
    bool leftYAxis;
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
    void addSignalFromJSON (const QJsonObject & graphSetting);
    void addAxesFromJSON (const QJsonObject & axesSetting);
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
