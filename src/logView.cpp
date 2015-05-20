#include "logView.h"
#include <QTime>
#include <QVBoxLayout>

LogView::LogView(QWidget *parent) :
    QWidget(parent)
{
    /*Main Layout*/
    logBox =  new QTextEdit;
    logBox->setReadOnly(true);

    QVBoxLayout * mainLayout = new QVBoxLayout();
    mainLayout->addWidget(logBox);

    setLayout(mainLayout);
}

void LogView::appendMessage(const QString &msg, Shared::MessageLevel msgLevel){
    if (msgLevel==Shared::information){
        logBox->setTextColor(Qt::black);
        emit sendMessageToFile(QTime::currentTime().toString("hh:mm:ss.zzz")+"\t"
                               +"Information:> "+msg+"\r\n");
    }
    else if (msgLevel==Shared::warning){
        logBox->setTextColor(Qt::blue);
        emit sendMessageToFile(QTime::currentTime().toString("hh:mm:ss.zzz")+"\t"
                               +"Warning:> "+msg+"\r\n");
    }
    else if (msgLevel==Shared::critical){
        logBox->setTextColor(Qt::red);
        emit sendMessageToFile(QTime::currentTime().toString("hh:mm:ss.zzz")+"\t"
                               +"Critical:> "+msg+"\r\n");
    }

    if (msgLevel!=Shared::empty)
    logBox->append(QTime::currentTime().toString("hh:mm:ss.zzz")+"\t"+msg);
    else
        logBox->append("");

}
