#ifndef LOGVIEW_H
#define LOGVIEW_H

#include <QWidget>
#include <QTextEdit>
#include "shared.h"

class LogView : public QWidget
{
    Q_OBJECT
public:
    explicit LogView(QWidget *parent = 0);
    
signals:
    void sendMessageToFile(const QString &);

public slots:
    void appendMessage(const QString & msg, Shared::MessageLevel msgLevel);
private:
    QTextEdit * logBox;
};

#endif // LOGVIEW_H
