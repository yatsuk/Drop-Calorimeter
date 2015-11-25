#include "logView.h"
#include "furnace.h"
#include <QTime>
#include <QEvent>
#include <QKeyEvent>
#include <QVBoxLayout>
#include <QHBoxLayout>

LogView::LogView(QWidget *parent) :
    QWidget(parent)
{
    logBox =  new QTextEdit;
    logBox->setReadOnly(true);

    userMessageEdit = new QLineEdit;
    sendUserMessageButton = new QPushButton (tr("Отправить"));
    QHBoxLayout * userMessageLayout = new QHBoxLayout();
    userMessageLayout->addWidget(userMessageEdit,1);
    userMessageLayout->addWidget(sendUserMessageButton);

    QVBoxLayout * mainLayout = new QVBoxLayout();
    mainLayout->addWidget(logBox);
    mainLayout->addLayout(userMessageLayout);

    setLayout(mainLayout);

    connect (sendUserMessageButton,SIGNAL(clicked()),this,SLOT(sendUserMessageButtonClicked()));
    userMessageEdit->installEventFilter(this);
}

void LogView::setElapsedTimer (QElapsedTimer * elapsedTimer)
{
    _elapsedTimer = elapsedTimer;
}

void LogView::appendMessage(const QString &msg, Shared::MessageLevel msgLevel){
    QString elapsedTime = "("+QString::number(Furnace::instance()->getElapsedTime()/1000.0)+")";

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
    else if (msgLevel==Shared::userMessage){
        logBox->setTextColor(Qt::gray);
        emit sendMessageToFile(QTime::currentTime().toString("hh:mm:ss.zzz")+ elapsedTime + "\t"
                               +"User-Message:> "+msg+"\r\n");
    }

    if (msgLevel!=Shared::empty)
        logBox->append(QTime::currentTime().toString("hh:mm:ss.zzz")+ elapsedTime + "\t"+msg);
    else
        logBox->append("");

}

void LogView::sendUserMessageButtonClicked()
{
    if (userMessageEdit->text().isEmpty())
        return;

    appendMessage(userMessageEdit->text(),Shared::userMessage);
    userMessageEdit->clear();
}

bool LogView::eventFilter(QObject * target, QEvent * event)
{
    if (target == userMessageEdit) {
        if (event->type() == QEvent::KeyPress) {
            QKeyEvent *keyEvent = (QKeyEvent *)event;
            if (keyEvent->key() == Qt::Key_Enter ||
                    keyEvent->key() == Qt::Key_Return) {
                sendUserMessageButtonClicked();
                return true;
            }
        }
    }
    return QWidget::eventFilter(target, event);
}
