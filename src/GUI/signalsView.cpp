#include "signalsView.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QFont>
#include <QDebug>

SignalsView::SignalsView(QWidget *parent) :
    QWidget(parent)
{
    furnaceSignalsView = new FurnaceSignalsView;

    layout = new QVBoxLayout;
    layout->addWidget(furnaceSignalsView);
    layout->addStretch();

    setLayout(layout);

    addSignal("ThermostatDiffTemperature");
    addSignal("ThermostatTemperature");
    addSignal("Resistance");
}

void SignalsView::addSignal(const QString &signalName){

    QGroupBox * box = new QGroupBox;

    box->setFont(QFont("Times", 16, QFont::Bold,true));

    if (signalName=="Resistance"){
        box->setTitle(tr("Сопротивление МТС блока"));
    }
    else if (signalName=="ThermostatTemperature"){
        box->setTitle(tr("Температура термостата"));
    }
    else if (signalName=="ThermostatDiffTemperature"){
        box->setTitle(tr("Дифф. тем-ра термостата"));
    }

    QLabel * label = new QLabel;
    label->setObjectName(signalName);
    labels.append(label);

    QVBoxLayout * boxLayout = new QVBoxLayout;
    boxLayout->addWidget(label,0,Qt::AlignRight);

    box->setLayout(boxLayout);

    layout->insertWidget(1,box);
}

void SignalsView::addValue(TerconData terconData){
    furnaceSignalsView->setTemperature(terconData);

    if (terconData.deviceNumber==1&&terconData.channel==1){
        for (int i =0;i <labels.size();++i){
            QLabel * label = labels.at(i);
            if (label->objectName()=="Resistance")
                label->setText(tr("<H1> %1 Ом</H1>").arg(QString::number(terconData.value,'f',3)));
        }
    }
    else if (terconData.deviceNumber==2&&terconData.channel==2){
        for (int i =0;i <labels.size();++i){
            QLabel * label = labels.at(i);
            if (label->objectName()=="ThermostatTemperature")
                label->setText(tr("<H1> %1 %2C</H1>").arg(QString::number(terconData.value,'f',3)).arg(QChar(176)));
        }
    }
    else if (terconData.deviceNumber==2&&terconData.channel==1){
        for (int i =0;i <labels.size();++i){
            QLabel * label = labels.at(i);
            if (label->objectName()=="ThermostatDiffTemperature")
                label->setText(tr("<H1> %1 мВ</H1>").arg(QString::number(terconData.value,'f',3)));
        }
    }
}

void SignalsView::updateState(const QJsonObject & json)
{
    furnaceSignalsView->updateState(json);
}






FurnaceSignalsView::FurnaceSignalsView(QWidget *parent) :
    QGroupBox(parent)
{   
    QFont bigFont;
    bigFont.setPointSize(10);
    setFont(bigFont);

    setTitle(tr("Печь калориметра"));
    sampleTemperatureLabel = new QLabel;
    sampleTemperatureLabel->hide();
    sampleTemperatureLabel->setText(tr("Температура обазца"));
    sampleTemperatureValueLabel = new QLabel;
    sampleTemperatureValueLabel->hide();
    QHBoxLayout * sampleTemperatureLayout = new QHBoxLayout;
    sampleTemperatureLayout->addWidget(sampleTemperatureLabel);
    sampleTemperatureLayout->addStretch(1);
    sampleTemperatureLayout->addWidget(sampleTemperatureValueLabel);

    furnaceInertBlockTemperatureLabel = new QLabel;
    furnaceInertBlockTemperatureLabel->hide();
    furnaceInertBlockTemperatureLabel->setText(tr("Температура выравнивающего блока"));
    furnaceInertBlockTemperatureValueLabel = new QLabel;
    furnaceInertBlockTemperatureValueLabel->hide();
    QHBoxLayout * furnaceInertBlockTemperatureLayout = new QHBoxLayout;
    furnaceInertBlockTemperatureLayout->addWidget(furnaceInertBlockTemperatureLabel);
    furnaceInertBlockTemperatureLayout->addStretch(1);
    furnaceInertBlockTemperatureLayout->addWidget(furnaceInertBlockTemperatureValueLabel);

    mainHeater = new HeaterSignalsView(tr("Основной нагреватель"));
    upHeater = new HeaterSignalsView(tr("Верхний охранный нагреватель"));
    downHeater = new HeaterSignalsView(tr("Нижний охранный нагреватель"));

    QVBoxLayout * mainLayout = new QVBoxLayout;
    mainLayout->addLayout(sampleTemperatureLayout);
    mainLayout->addLayout(furnaceInertBlockTemperatureLayout);
    mainLayout->addWidget(mainHeater);
    mainLayout->addWidget(upHeater);
    mainLayout->addWidget(downHeater);
    setLayout(mainLayout);
}

void FurnaceSignalsView::setTemperature(TerconData terconData)
{
    if (terconData.deviceNumber==5&&terconData.channel==1){
        upHeater->setTemperature(terconData.value);
    }
    else if (terconData.deviceNumber==5&&terconData.channel==2){
        mainHeater->setTemperature(terconData.value);
    }
    else if (terconData.deviceNumber==5&&terconData.channel==3){
        downHeater->setTemperature(terconData.value);
    }
    else if (terconData.deviceNumber==5&&terconData.channel==4){
        if (furnaceInertBlockTemperatureLabel->isHidden()){
            furnaceInertBlockTemperatureLabel->show();
            furnaceInertBlockTemperatureValueLabel->show();
        }
        furnaceInertBlockTemperatureValueLabel->setText(tr("%1%2С")
                                                        .arg(QString::number(terconData.value,'f',1))
                                                        .arg(QChar(176)));
    }
    else if (terconData.deviceNumber==5&&terconData.channel==5){
        if (sampleTemperatureLabel->isHidden()){
            sampleTemperatureLabel->show();
            sampleTemperatureValueLabel->show();
        }
        sampleTemperatureValueLabel->setText(tr("%1%2С")
                                             .arg(QString::number(terconData.value,'f',1))
                                             .arg(QChar(176)));
    }
}

void FurnaceSignalsView::updateState(const QJsonObject & json)
{
    if (json["sender"].toString()== "regulator-main-heater"){
        mainHeater->updateState(json);
    } else if (json["sender"].toString()== "regulator-up-heater"){
        upHeater->updateState(json);
    } else if (json["sender"].toString()== "regulator-down-heater"){
        downHeater->updateState(json);
    }
}



HeaterSignalsView::HeaterSignalsView(const QString & heaterName, QWidget *parent) :
    QWidget(parent)
{
    name_ = heaterName;
    defaultColorOn1LegIndicator = QColor(0,255,0);
    defaultColorOn2LegIndicator = QColor(0,192,0);
    defaultColorOff1LegIndicator = QColor(0,28,0);
    defaultColorOff2LegIndicator = QColor(0,128,0);

    statusHeaterLed = new QLedIndicator();
    statusHeaterLed->setCheckable(false);
    heaterLabel = new QLabel(heaterName);
    temperatureLabel = new QLabel;

    QHBoxLayout * hLayout = new QHBoxLayout;
    hLayout->addWidget(statusHeaterLed);
    hLayout->addWidget(heaterLabel);
    hLayout->addStretch(1);
    hLayout->addWidget(temperatureLabel);


    heaterGroupBox = new QGroupBox;
    heaterGroupBox->hide();

    regulatorMode = new QLabel;
    outPower = new QLabel;
    setPoint = new QLabel;
    setPoint->hide();
    delta = new QLabel;
    delta->hide();

    QHBoxLayout * h2Layout = new QHBoxLayout;
    h2Layout->addWidget(setPoint);
    h2Layout->addStretch(1);
    h2Layout->addWidget(delta);

    leftTimeProgressBarViewMode = false;
    progressBar = new QProgressBar();
    progressBar->setAlignment(Qt::AlignCenter);
    progressBar->installEventFilter(this);
    progressBar->hide();
    segmentInfoLabel = new QLabel();
    segmentInfoLabel->hide();

    QVBoxLayout * groupBoxLayout = new QVBoxLayout;
    groupBoxLayout->addWidget(regulatorMode);
    groupBoxLayout->addLayout(h2Layout);
    groupBoxLayout->addWidget(outPower);
    groupBoxLayout->addWidget(progressBar);
    groupBoxLayout->addWidget(segmentInfoLabel);

    heaterGroupBox->setLayout(groupBoxLayout);


    QVBoxLayout * mainLayout = new QVBoxLayout;
    mainLayout->addLayout(hLayout);
    mainLayout->addWidget(heaterGroupBox);
    setLayout(mainLayout);
}

void HeaterSignalsView::setTemperature(double temperature)
{
    temperatureLabel->setText(tr("%1%2С").arg(QString::number(temperature,'f',1)).arg(QChar(176)));
}

void HeaterSignalsView::updateState(const QJsonObject & json)
{
    if (json["enable"].toBool()&& !json["emergency-stop"].toBool()){
        statusHeaterLed->setOffColor1(defaultColorOn1LegIndicator);
        statusHeaterLed->setOffColor2(defaultColorOn2LegIndicator);
        heaterGroupBox->show();
        outPower->setText(tr("Выходная мощность: %1%")
                          .arg(QString::number(json["out-power"].toDouble(),'f',1)));
        QString mode = json["mode"].toString();
        if (mode=="automatic"){
            regulatorMode->setText(tr("Тип управления: автоматический"));
            setPoint->setText(tr("Уставка: %1%2С")
                              .arg(QString::number(json["set-point"].toDouble(),'f',1)).arg(QChar(176)));
            setPoint->show();
            changeColorDeltaTemperatureLabel(json["delta"].toDouble());
            delta->show();

            QJsonObject jsonTemperatureSegment = json["temperature-segment"].toObject();
            QString segmentTitle = jsonTemperatureSegment["type"].toString();
            if (!segmentTitle.isEmpty()){
                if (segmentTitle=="Изотерма"){
                    segmentInfoLabel->setText(segmentTitle);
                } else {
                    segmentInfoLabel->setText(tr("%1 %2 К/мин")
                                              .arg(segmentTitle)
                                              .arg(QString::number(jsonTemperatureSegment["velocity"].toDouble())));
                }
                segmentInfoLabel->show();

                durationTimeProgress = jsonTemperatureSegment["duration"].toDouble();
                elapsedTimeProgress = jsonTemperatureSegment["elapsed-time"].toDouble();
                progressBar->setMaximum(durationTimeProgress);
                progressBar->setValue(elapsedTimeProgress);
                setProgressBarText();
                progressBar->show();
            }

        } else if (mode=="manual"){
            regulatorMode->setText(tr("Тип управления: ручной"));
            setPoint->hide();
            delta->hide();
            progressBar->hide();
            segmentInfoLabel->hide();

        } else if (mode=="programPower"){
            regulatorMode->setText(tr("Тип управления: программируемая мощность"));
            setPoint->hide();
            delta->hide();
            progressBar->hide();
            segmentInfoLabel->hide();

        } else if (mode=="stopCurrentTemperature"){
            regulatorMode->setText(tr("Тип управления: постоянная температура"));
            setPoint->setText(tr("Уставка: %1%2С")
                              .arg(QString::number(json["set-point"].toDouble(),'f',1)).arg(QChar(176)));
            setPoint->show();
            changeColorDeltaTemperatureLabel(json["delta"].toDouble());
            delta->show();
            progressBar->hide();
            segmentInfoLabel->hide();

        } else if (mode=="constVelocity"){
            regulatorMode->setText(tr("Тип управления: постоянная скорость"));
            setPoint->setText(tr("Уставка: %1%2С")
                              .arg(QString::number(json["set-point"].toDouble(),'f',1)).arg(QChar(176)));
            setPoint->show();
            changeColorDeltaTemperatureLabel(json["delta"].toDouble());
            delta->show();
            progressBar->hide();
            segmentInfoLabel->hide();

        } else if (mode=="constValue"){
            regulatorMode->setText(tr("Тип управления: постоянная разница"));
            setPoint->setText(tr("Уставка: %1%2С")
                              .arg(QString::number(json["set-point"].toDouble(),'f',1)).arg(QChar(176)));
            setPoint->show();
            changeColorDeltaTemperatureLabel(json["delta"].toDouble());
            delta->show();
            progressBar->hide();
            segmentInfoLabel->hide();
        }
    } else {
        statusHeaterLed->setOffColor1(defaultColorOff1LegIndicator);
        statusHeaterLed->setOffColor2(defaultColorOff2LegIndicator);
        heaterGroupBox->hide();
    }
    if (json["emergency-stop"].toBool()){
        statusHeaterLed->setOffColor1(Qt::red);
        statusHeaterLed->setOffColor2(Qt::red);
        heaterGroupBox->hide();
    }
}

void HeaterSignalsView::changeColorDeltaTemperatureLabel(double deltaTemperature)
{
    double absDelta = qAbs(deltaTemperature);
    if (absDelta <= 0.2){
        delta->setText(tr("<font color='green'>\u0394: %1%2С</font>")
                       .arg(QString::number(deltaTemperature,'f',1)).arg(QChar(176)));
    } else if (absDelta <= 1) {
        delta->setText(tr("<font color='blue'>\u0394: %1%2С</font>")
                       .arg(QString::number(deltaTemperature,'f',1)).arg(QChar(176)));
    } else {
        delta->setText(tr("<font color='red'>\u0394: %1%2С</font>")
                       .arg(QString::number(deltaTemperature,'f',1)).arg(QChar(176)));
    }

}

void HeaterSignalsView::setProgressBarText()
{
    if (leftTimeProgressBarViewMode){
        progressBar->setFormat(tr("осталась %1 мин, всего %2 мин, %p%")
                               .arg(QString::number((durationTimeProgress - elapsedTimeProgress)/60,'f',0))
                               .arg(QString::number(durationTimeProgress/60,'f',0)));
    } else {
        progressBar->setFormat(tr("прошло %1 мин, всего %2 мин, %p%")
                               .arg(QString::number(elapsedTimeProgress/60,'f',0))
                               .arg(QString::number(durationTimeProgress/60,'f',0)));
    }
}

bool HeaterSignalsView::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == progressBar) {
        if (event->type() == QEvent::MouseButtonPress) {
            QMouseEvent * mouseEvent = (QMouseEvent *)event;
            if (mouseEvent->button()== Qt::LeftButton){
                progressBarClicked();
                return true;
            }
        }
    }
    return QObject::eventFilter(obj, event);
}

void HeaterSignalsView::progressBarClicked()
{
    leftTimeProgressBarViewMode=!leftTimeProgressBarViewMode;
    setProgressBarText();
}

