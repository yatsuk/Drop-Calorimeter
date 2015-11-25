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
    plot->xAxis->setLabel(tr("время, сек"));
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
    plot->plotLayout()->addElement(0, 0, new QCPPlotTitle(plot, text));
}

void ChartWidget::setYLeftAxisTitle (const QString & title)
{
    plot->yAxis->setLabel(title);
}

void ChartWidget::setYRigthAxisTitle (const QString & title)
{
    plot->yAxis2->setLabel(title);
}

void ChartWidget::addSignal(int devNumber, int chNumber, bool leftYAxis, QColor color, const QString &legend)
{

    if (!leftYAxis){
        plot->addGraph(plot->xAxis,plot->yAxis2);
        plot->yAxis2->setVisible(true);
    } else {
        plot->addGraph();
    }


    plot->graph()->setPen(QPen(color));
    plot->graph()->setName(legend);

    GraphData2 graphData;
    graphData.deviceNumber = devNumber;
    graphData.channelNumber = chNumber;
    graphData.leftYAxis = leftYAxis;
    graphsData.append(graphData);

    plot->replot();
}

void ChartWidget::addDataTercon(TerconData terconData){
    for (int i = 0; i < graphsData.size(); ++i){
        if((terconData.channel==graphsData[i].channelNumber)&&(terconData.deviceNumber==graphsData[i].deviceNumber)){
            plot->graph(i)->addData(terconData.time/1000.0, terconData.value);
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
