#include "logView.h"
#include "furnace.h"
#include <QTime>
#include <QVBoxLayout>

LogView::LogView(QWidget *parent) :
    QWidget(parent)
{
    _elapsedTimer = Furnace::instance()->getElapsedTimer();
    /*Main Layout*/
    logBox =  new QTextEdit;
    logBox->setReadOnly(true);

    QVBoxLayout * mainLayout = new QVBoxLayout();
    mainLayout->addWidget(logBox);

    setLayout(mainLayout);
}

void LogView::setElapsedTimer (QElapsedTimer * elapsedTimer)
{
    _elapsedTimer = elapsedTimer;
}

void LogView::appendMessage(const QString &msg, Shared::MessageLevel msgLevel){
    QString elapsedTime;
    if (_elapsedTimer->isValid())
        elapsedTime = "("+QString::number(_elapsedTimer->elapsed()/1000.0)+")";

    if (msgLevel==Shared::information){
        logBox->setTextColor(Qt::black);
        emit sendMessageToFile(QTime::currentTime().toString("hh:mm:ss.zzz")+ elapsedTime + "\t"
                               +"Information:> "+msg+"\r\n");
    }
    else if (msgLevel==Shared::warning){
        logBox->setTextColor(Qt::blue);
        emit sendMessageToFile(QTime::currentTime().toString("hh:mm:ss.zzz")+ elapsedTime + "\t"
                               +"Warning:> "+msg+"\r\n");
    }
    else if (msgLevel==Shared::critical){
        logBox->setTextColor(Qt::red);
        emit sendMessageToFile(QTime::currentTime().toString("hh:mm:ss.zzz")+ elapsedTime + "\t"
                               +"Critical:> "+msg+"\r\n");
    }

    if (msgLevel!=Shared::empty)
        logBox->append(QTime::currentTime().toString("hh:mm:ss.zzz")+ elapsedTime + "\t"+msg);
    else
        logBox->append("");

}
