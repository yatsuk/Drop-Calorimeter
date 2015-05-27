#ifndef LOGVIEW_H
#define LOGVIEW_H

#include <QWidget>
#include <QTextEdit>
#include <QElapsedTimer>
#include "shared.h"

class LogView : public QWidget
{
    Q_OBJECT
public:
    explicit LogView(QWidget *parent = 0);
    void setElapsedTimer (QElapsedTimer * elapsedTimer);
    
signals:
    void sendMessageToFile(const QString &);

public slots:
    void appendMessage(const QString & msg, Shared::MessageLevel msgLevel);
private:
    QTextEdit * logBox;
    QElapsedTimer * _elapsedTimer;
};

#endif // LOGVIEW_H
