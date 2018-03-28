#ifndef LOGVIEW_H
#define LOGVIEW_H

#include <QWidget>
#include <QTextEdit>
#include <QLineEdit>
#include <QPushButton>
#include <QElapsedTimer>
#include "src/shared.h"

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

private slots:
    void sendUserMessageButtonClicked();

protected:
    bool eventFilter(QObject * target, QEvent * event);

private:
    QTextEdit * logBox;
    QLineEdit * userMessageEdit;
    QPushButton * sendUserMessageButton;
    QElapsedTimer * _elapsedTimer;
};

#endif // LOGVIEW_H
