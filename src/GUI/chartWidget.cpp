#include "chartWidget.h"
#include <QtWidgets/QVBoxLayout>
#include <QDebug>

ChartWidget::ChartWidget(QWidget *parent) : QWidget(parent)
{
    initGraph();

    QVBoxLayout * vLayout = new QVBoxLayout;
    vLayout->addWidget(plot);

    setMinimumSize(400,350);
    setLayout(vLayout);
}

ChartWidget::~ChartWidget()
{

}

void ChartWidget::initGraph()
{
    xAxesClicked = false;
    yAxesClicked = false;

    plot = new AdvancedQCustomPlot;
    plot->legend->setVisible(true);
    plot->axisRect()->insetLayout()->setInsetAlignment(0, Qt::AlignLeft|Qt::AlignBottom);
    //plot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectAxes |
    //                                QCP::iSelectLegend | QCP::iSelectPlottables);
    plot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);

    //plot->setContextMenuPolicy(Qt::CustomContextMenu);
    //connect(plot, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(contextMenuRequest(QPoint)));

    //plot->setNotAntialiasedElements(QCP::aeAll);
}

void ChartWidget::setPlotTitle(const QString & text)
{
    plot->plotLayout()->insertRow(0);
    plot->plotLayout()->addElement(0, 0, new QCPTextElement(plot, text, QFont("sans", 12, QFont::Bold)));
}

void ChartWidget::addSignalFromJSON (const json & graphSetting)
{

    GraphData2 graphData;

    if (graphSetting["YAxes"].get<std::string>().c_str()==plot->yAxis2->objectName()){
        plot->addGraph(plot->xAxis,plot->yAxis2);
        graphData.yAxis2 = true;
        connect(plot->xAxis, SIGNAL(rangeChanged(QCPRange)), plot->xAxis2, SLOT(setRange(QCPRange)));
    } else {
        plot->addGraph();
        graphData.yAxis2 = false;
        connect(plot->xAxis, SIGNAL(rangeChanged(QCPRange)), plot->xAxis2, SLOT(setRange(QCPRange)));
        connect(plot->yAxis, SIGNAL(rangeChanged(QCPRange)), plot->yAxis2, SLOT(setRange(QCPRange)));
    }

    plot->graph()->setPen(QPen(QColor(graphSetting["color"].get<std::string>().c_str())));
    plot->graph()->setName(graphSetting["title"].get<std::string>().c_str());

    graphData.sourceId = graphSetting["source"].get<std::string>().c_str();
    graphData.multiplier = graphSetting["multiplier"];
    graphsData.append(graphData);

    plot->replot();
}

void ChartWidget::addAxesFromJSON (const json &axesSetting)
{
    if (axesSetting["orientation"].get<std::string>().c_str() == "left"){
        plot->yAxis->setObjectName(axesSetting["id"].get<std::string>().c_str());
        plot->yAxis->setLabel(axesSetting["title"].get<std::string>().c_str());
        plot->yAxis2->setVisible(true);
        plot->yAxis2->setTickLabels(false);
        plot->yAxis->setNumberFormat("f");
        plot->yAxis->setNumberPrecision(0);
    } else if (axesSetting["orientation"].get<std::string>().c_str() == "bottom"){
        plot->xAxis->setObjectName(axesSetting["id"].get<std::string>().c_str());
        plot->xAxis->setLabel(axesSetting["title"].get<std::string>().c_str());
        plot->xAxis2->setVisible(true);
        plot->xAxis2->setTickLabels(false);
    } else if (axesSetting["orientation"].get<std::string>().c_str() == "right"){
        plot->yAxis2->setObjectName(axesSetting["id"].get<std::string>().c_str());
        plot->yAxis2->setLabel(axesSetting["title"].get<std::string>().c_str());
        plot->xAxis2->setVisible(true);
        plot->xAxis2->setTickLabels(true);
        plot->yAxis2->setNumberFormat("f");
        plot->yAxis2->setNumberPrecision(0);
    }
}

void ChartWidget::addDataTercon(TerconData terconData){
    for (int i = 0; i < graphsData.size(); ++i){
        if(terconData.id==graphsData[i].sourceId){
            double delta = plot->yAxis->range().upper - plot->yAxis->range().lower;
            int precision = round(log10(delta));

            if (!graphsData[i].yAxis2){
            if (precision < 1){
                    plot->yAxis->setNumberPrecision(abs(precision) + 1);
                } else {
                    plot->yAxis->setNumberPrecision(0);
                }
            } else {
                if (precision == 1) {
                    plot->yAxis2->setNumberPrecision(1);
                } else if (precision < 1){
                    plot->yAxis2->setNumberPrecision(abs(precision) + 1);
                } else {
                    plot->yAxis2->setNumberPrecision(0);
                }
            }

            plot->graph(i)->addData(terconData.time/1000.0, terconData.value*graphsData[i].multiplier);
            if (!plot->isZoomed()){
                plot->rescaleAxes();
            }
            plot->replot();
        }
    }
}

/*void ChartWidget::contextMenuRequest(QPoint pos)
{
    QMenu *menu = new QMenu(this);
    menu->setAttribute(Qt::WA_DeleteOnClose);

    if (plot->legend->selectTest(pos, false) >= 0) // context menu on legend requested
    {
        menu->addAction("Move to top left", this, SLOT(moveLegend()))->setData((int)(Qt::AlignTop|Qt::AlignLeft));
        menu->addAction("Move to top center", this, SLOT(moveLegend()))->setData((int)(Qt::AlignTop|Qt::AlignHCenter));
        menu->addAction("Move to top right", this, SLOT(moveLegend()))->setData((int)(Qt::AlignTop|Qt::AlignRight));
        menu->addAction("Move to bottom right", this, SLOT(moveLegend()))->setData((int)(Qt::AlignBottom|Qt::AlignRight));
        menu->addAction("Move to bottom center", this, SLOT(moveLegend()))->setData((int)(Qt::AlignBottom|Qt::AlignHCenter));
        menu->addAction("Move to bottom left", this, SLOT(moveLegend()))->setData((int)(Qt::AlignBottom|Qt::AlignLeft));
        menu->addAction("Move to center left", this, SLOT(moveLegend()))->setData((int)(Qt::AlignVCenter|Qt::AlignLeft));
        menu->addAction("Move to center right", this, SLOT(moveLegend()))->setData((int)(Qt::AlignVCenter|Qt::AlignRight));
    }

    menu->popup(plot->mapToGlobal(pos));
}

void ChartWidget::moveLegend()
{
    if (QAction* contextAction = qobject_cast<QAction*>(sender())) // make sure this slot is really called by a context menu action, so it carries the data we need
    {
        bool ok;
        int dataInt = contextAction->data().toInt(&ok);
        if (ok)
        {
            plot->axisRect()->insetLayout()->setInsetAlignment(0, (Qt::Alignment)dataInt);
            //plot->replot();
        }
    }
}*/

AdvancedQCustomPlot::AdvancedQCustomPlot()
{
    isZoomed_ = false;
}

bool AdvancedQCustomPlot::isZoomed()
{
    return isZoomed_;
}

void AdvancedQCustomPlot::wheelEvent(QWheelEvent *event)
{
    if (event->modifiers() == Qt::ShiftModifier){
        axisRect()->setRangeZoom(Qt::Vertical);
    } else if (event->modifiers() == Qt::ControlModifier){
        axisRect()->setRangeZoom(Qt::Horizontal);
    } else {
        axisRect()->setRangeZoom(Qt::Vertical | Qt::Horizontal);
    }
    isZoomed_ = true;
    QCustomPlot::wheelEvent(event);
}

void AdvancedQCustomPlot::mousePressEvent(QMouseEvent *event)
{
    if(event->buttons() & Qt::RightButton)
    {
        isZoomed_ = false;
        rescaleAxes(true);
        replot();
    }
    QCustomPlot::mousePressEvent(event);

}
