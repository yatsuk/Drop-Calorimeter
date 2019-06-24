#include "signalsView.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QFont>
#include <QtMath>
#include <QDebug>
#include "src/furnace.h"

FurnaceSignalsView::FurnaceSignalsView(QWidget *parent) :
    QGroupBox(parent)
{   
    QFont bigFont;
    bigFont.setPointSize(10);
    setFont(bigFont);

    QFont labelValueFont;
    labelValueFont.setPointSize(20);

    setTitle(tr("Печь калориметра"));
    sampleTemperatureLabel = new QLabel;
    sampleTemperatureLabel->hide();
    sampleTemperatureLabel->setText(tr("Температура обазца"));
    sampleTemperatureValueLabel = new QLabel;
    sampleTemperatureValueLabel->hide();
    sampleTemperatureValueLabel->setFont(labelValueFont);
    QHBoxLayout * sampleTemperatureLayout = new QHBoxLayout;
    sampleTemperatureLayout->addWidget(sampleTemperatureLabel);
    sampleTemperatureLayout->addStretch(1);
    sampleTemperatureLayout->addWidget(sampleTemperatureValueLabel);

    furnaceInertBlockTemperatureLabel = new QLabel;
    furnaceInertBlockTemperatureLabel->hide();
    furnaceInertBlockTemperatureLabel->setText(tr("Температура вырав. блока"));
    furnaceInertBlockTemperatureValueLabel = new QLabel;
    furnaceInertBlockTemperatureValueLabel->hide();
    furnaceInertBlockTemperatureValueLabel->setFont(labelValueFont);
    QHBoxLayout * furnaceInertBlockTemperatureLayout = new QHBoxLayout;
    furnaceInertBlockTemperatureLayout->addWidget(furnaceInertBlockTemperatureLabel);
    furnaceInertBlockTemperatureLayout->addStretch(1);
    furnaceInertBlockTemperatureLayout->addWidget(furnaceInertBlockTemperatureValueLabel);

    coldWaterTemperatureLabel = new QLabel;
    coldWaterTemperatureLabel->hide();
    coldWaterTemperatureLabel->setText(tr("Температура холодной воды"));
    coldWaterTemperatureValueLabel = new QLabel;
    coldWaterTemperatureValueLabel->hide();
    coldWaterTemperatureValueLabel->setFont(labelValueFont);
    QHBoxLayout * coldWaterTemperatureLayout = new QHBoxLayout;
    coldWaterTemperatureLayout->addWidget(coldWaterTemperatureLabel);
    coldWaterTemperatureLayout->addStretch(1);
    coldWaterTemperatureLayout->addWidget(coldWaterTemperatureValueLabel);

    mainHeater = new HeaterSignalsView(tr("Основной нагреватель"));
    upHeater = new HeaterSignalsView(tr("Верхний нагреватель"));
    downHeater = new HeaterSignalsView(tr("Нижний нагреватель"));

    QVBoxLayout * mainLayout = new QVBoxLayout;
    mainLayout->addLayout(sampleTemperatureLayout);
    mainLayout->addLayout(furnaceInertBlockTemperatureLayout);
    mainLayout->addWidget(mainHeater);
    mainLayout->addWidget(upHeater);
    mainLayout->addWidget(downHeater);
    mainLayout->addLayout(coldWaterTemperatureLayout);
    mainLayout->addStretch();
    setLayout(mainLayout);

    widgetRegulatorFurnace = new WidgetRegulatorFurnace(this);
    widgetRegulatorFurnace->setRegulator(Furnace::instance()->regulatorFurnace());

    additionalHeatersWidget = new AdditionalHeatersWidget;

    connect(upHeater,SIGNAL(statusLedClicked()),this,SLOT(showAdditionalRegulatorSettingsWidget()));
    connect(downHeater,SIGNAL(statusLedClicked()),this,SLOT(showAdditionalRegulatorSettingsWidget()));
    connect(mainHeater,SIGNAL(statusLedClicked()),this,SLOT(showMainRegulatorSettingsWidget()));
}

void FurnaceSignalsView::setTemperature(TerconData terconData)
{
    if (terconData.id == "{33bc6e29-4e26-41a6-85f1-cfa0bc5525b9}"){
        upHeater->setTemperature(terconData.value);
    }
    else if (terconData.id == "{a5f14434-bbc4-435e-be15-a7ad91de2701}"){
        mainHeater->setTemperature(terconData.value);
    }
    else if (terconData.id == "{7cb4d773-5b1d-4060-b316-24045308317f}"){
        downHeater->setTemperature(terconData.value);
    }
    else if (terconData.id == "{425f2f72-f773-4f12-8914-fccffacabcfb}"){
        if (furnaceInertBlockTemperatureLabel->isHidden()){
            furnaceInertBlockTemperatureLabel->show();
            furnaceInertBlockTemperatureValueLabel->show();
        }
        furnaceInertBlockTemperatureValueLabel->setText(tr("%1%2С")
                                                        .arg(QString::number(terconData.value,'f',1))
                                                        .arg(QChar(176)));
    }
    else if (terconData.id == "{c922039f-afed-4447-9a04-1067f27b2b49}"){
        if (sampleTemperatureLabel->isHidden()){
            sampleTemperatureLabel->show();
            sampleTemperatureValueLabel->show();
        }
        sampleTemperatureValueLabel->setText(tr("%1%2С")
                                             .arg(QString::number(terconData.value,'f',1))
                                             .arg(QChar(176)));
    }
    else if (terconData.id == "{5608ce90-8515-482c-a25b-c5403f715988}"){
        if (coldWaterTemperatureLabel->isHidden()){
            coldWaterTemperatureLabel->show();
            coldWaterTemperatureValueLabel->show();
        }
        coldWaterTemperatureValueLabel->setText(tr("%1%2С")
                                             .arg(QString::number(terconData.value,'f',1))
                                             .arg(QChar(176)));
    }
}

void FurnaceSignalsView::updateState(const json &state)
{
    if (QString::compare(state["sender"].get<std::string>().c_str(), "regulator-main-heater") == 0){
        mainHeater->updateState(state);
    } else if (QString::compare(state["sender"].get<std::string>().c_str(), "regulator-up-heater") == 0){
        upHeater->updateState(state);
    } else if (QString::compare(state["sender"].get<std::string>().c_str(), "regulator-down-heater") == 0){
        downHeater->updateState(state);
    }
}

void FurnaceSignalsView::showMainRegulatorSettingsWidget()
{
    widgetRegulatorFurnace->setWindowTitle(tr("Основной нагреватель"));
    widgetRegulatorFurnace->exec();
}

void FurnaceSignalsView::showAdditionalRegulatorSettingsWidget()
{
    additionalHeatersWidget->setWindowTitle(tr("Охранные нагреватели"));
    additionalHeatersWidget->exec();
}





HeaterSignalsView::HeaterSignalsView(const QString & heaterName, QWidget *parent) :
    QWidget(parent)
{
    valuePresision_ = 1;

    name_ = heaterName;
    defaultColorOn1LegIndicator = QColor(0,255,0);
    defaultColorOn2LegIndicator = QColor(0,192,0);
    defaultColorOff1LegIndicator = QColor(0,28,0);
    defaultColorOff2LegIndicator = QColor(0,128,0);


    QFont labelValueFont;
    labelValueFont.setPointSize(20);
    statusHeaterLed = new QLedIndicator();
    statusHeaterLed->setCheckable(false);
    statusHeaterLed->installEventFilter(this);
    heaterLabel = new QLabel(heaterName);
    temperatureLabel = new QLabel;
    temperatureLabel->setFont(labelValueFont);

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

    pLabel = new QLabel;
    dLabel = new QLabel;
    iLabel = new QLabel;
    QHBoxLayout * h3Layout = new QHBoxLayout;
    h3Layout->addWidget(pLabel);
    h3Layout->addWidget(dLabel);
    h3Layout->addWidget(iLabel);

    QVBoxLayout * groupBoxLayout = new QVBoxLayout;
    groupBoxLayout->addWidget(regulatorMode);
    groupBoxLayout->addLayout(h2Layout);
    groupBoxLayout->addWidget(outPower);
    groupBoxLayout->addWidget(progressBar);
    groupBoxLayout->addWidget(segmentInfoLabel);
    groupBoxLayout->addLayout(h3Layout);

    heaterGroupBox->setLayout(groupBoxLayout);


    QVBoxLayout * mainLayout = new QVBoxLayout;
    mainLayout->addLayout(hLayout);
    mainLayout->addWidget(heaterGroupBox);
    setLayout(mainLayout);
}

void HeaterSignalsView::setTemperature(double temperature)
{
    temperatureLabel->setText(tr("%1%2С").arg(QString::number(temperature,'f',valuePresision_)).arg(QChar(176)));
}

void HeaterSignalsView::updateState(const json &state)
{
    if (state["enable"]&& !state["emergency-stop"]){
        statusHeaterLed->setOffColor1(defaultColorOn1LegIndicator);
        statusHeaterLed->setOffColor2(defaultColorOn2LegIndicator);
        heaterGroupBox->show();
        outPower->setText(tr("Выходная мощность: %1%")
                          .arg(QString::number(state["out-power"],'f',1)));
        pLabel->setText(tr("P: %1%")
                        .arg(QString::number(state["P*delta"],'f',1)));
        iLabel->setText(tr("I: %1%")
                        .arg(QString::number(state["I*delta"],'f',2)));
        dLabel->setText(tr("D: %1%")
                        .arg(QString::number(state["D*delta"],'f',1)));

        QString mode = state["mode"].get<std::string>().c_str();
        if (mode=="automatic"){
            regulatorMode->setText(tr("Тип управления: автоматический"));
            setPoint->setText(tr("Уставка: %1%2С")
                              .arg(QString::number(state["set-point"],'f',1)).arg(QChar(176)));
            setPoint->show();
            changeColorDeltaTemperatureLabel(state["delta"]);
            delta->show();

            if (!state["temperature-segment"].is_null()){
                json jsonTemperatureSegment = state["temperature-segment"];
                QString segmentTitle = jsonTemperatureSegment["type"].get<std::string>().c_str();
                if (!segmentTitle.isEmpty()){
                    if (segmentTitle=="Изотерма"){
                        segmentInfoLabel->setText(segmentTitle);
                    } else {
                        segmentInfoLabel->setText(tr("%1 %2 К/мин")
                                                  .arg(segmentTitle)
                                                  .arg(QString::number(jsonTemperatureSegment["velocity"].get<double>())));
                    }
                    segmentInfoLabel->show();

                    durationTimeProgress = jsonTemperatureSegment["duration"];
                    elapsedTimeProgress = jsonTemperatureSegment["elapsed-time"];
                    progressBar->setMaximum(durationTimeProgress);
                    progressBar->setValue(elapsedTimeProgress);
                    setProgressBarText();
                    progressBar->show();
                }
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
                              .arg(QString::number(state["set-point"],'f',1)).arg(QChar(176)));
            setPoint->show();
            changeColorDeltaTemperatureLabel(state["delta"]);
            delta->show();
            progressBar->hide();
            segmentInfoLabel->hide();

        } else if (mode=="constVelocity"){
            regulatorMode->setText(tr("Тип управления: постоянная скорость"));
            setPoint->setText(tr("Уставка: %1%2С")
                              .arg(QString::number(state["set-point"],'f',1)).arg(QChar(176)));
            setPoint->show();
            changeColorDeltaTemperatureLabel(state["delta"]);
            delta->show();
            progressBar->hide();
            segmentInfoLabel->hide();

        } else if (mode=="constValue"){
            regulatorMode->setText(tr("Тип управления: постоянная разница"));
            setPoint->setText(tr("Уставка: %1%2С")
                              .arg(QString::number(state["set-point"],'f',1)).arg(QChar(176)));
            setPoint->show();
            changeColorDeltaTemperatureLabel(state["delta"]);
            delta->show();
            progressBar->hide();
            segmentInfoLabel->hide();
        }
    } else {
        statusHeaterLed->setOffColor1(defaultColorOff1LegIndicator);
        statusHeaterLed->setOffColor2(defaultColorOff2LegIndicator);
        heaterGroupBox->hide();
    }
    if (state["emergency-stop"]){
        statusHeaterLed->setOffColor1(Qt::red);
        statusHeaterLed->setOffColor2(Qt::red);
        heaterGroupBox->hide();
    }
}

void HeaterSignalsView::changeColorDeltaTemperatureLabel(double deltaTemperature)
{
    double absDelta = qAbs(deltaTemperature);
    if (absDelta <= 2/qPow(10,valuePresision_)){
        delta->setText(tr("<font color='green'>\u0394: %1%2С</font>")
                       .arg(QString::number(deltaTemperature,'f',valuePresision_)).arg(QChar(176)));
    } else if (absDelta <= 10/qPow(10,valuePresision_)) {
        delta->setText(tr("<font color='blue'>\u0394: %1%2С</font>")
                       .arg(QString::number(deltaTemperature,'f',valuePresision_)).arg(QChar(176)));
    } else {
        delta->setText(tr("<font color='red'>\u0394: %1%2С</font>")
                       .arg(QString::number(deltaTemperature,'f',valuePresision_)).arg(QChar(176)));
    }

}

void HeaterSignalsView::setProgressBarText()
{
    if (leftTimeProgressBarViewMode){
        progressBar->setFormat(tr("осталось %1 мин, всего %2 мин, %p%")
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
    if (obj == progressBar && event->type() == QEvent::MouseButtonPress) {
        QMouseEvent * mouseEvent = (QMouseEvent *)event;
        if (mouseEvent->button()== Qt::LeftButton){
            progressBarClicked();
            return true;
        }
    }
    if (obj == statusHeaterLed){
        if (event->type() == QEvent::Leave) {
            setCursor(Qt::ArrowCursor);
            return true;

        } else if (event->type() == QEvent::Enter){
            setCursor(Qt::PointingHandCursor);
            return true;

        } else if (event->type() == QEvent::MouseButtonPress){
            QMouseEvent * mouseEvent = (QMouseEvent *)event;
            if (mouseEvent->button()== Qt::LeftButton){
                emit statusLedClicked();
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





CalorimeterSignalsView::CalorimeterSignalsView(QWidget *parent) :
    QGroupBox(parent)
{
    QFont bigFont;
    bigFont.setPointSize(10);
    setFont(bigFont);

    QFont labelValueFont;
    labelValueFont.setPointSize(20);

    setTitle(tr("Калориметрический блок"));
    resistanceLabel = new QLabel(tr("Сопротивление МТС"));
    resistanceLabel->hide();
    resistanceValueLabel = new QLabel;
    resistanceValueLabel->hide();
    resistanceValueLabel->setFont(labelValueFont);
    QHBoxLayout * sampleTemperatureLayout = new QHBoxLayout;
    sampleTemperatureLayout->addWidget(resistanceLabel);
    sampleTemperatureLayout->addStretch(1);
    sampleTemperatureLayout->addWidget(resistanceValueLabel);

    diffTemperatureLabel = new QLabel(tr("Диф. тем-ра термостата"));
    diffTemperatureLabel->hide();
    diffTemperatureValueLabel = new QLabel;
    diffTemperatureValueLabel->hide();
    diffTemperatureValueLabel->setFont(labelValueFont);
    QHBoxLayout * furnaceInertBlockTemperatureLayout = new QHBoxLayout;
    furnaceInertBlockTemperatureLayout->addWidget(diffTemperatureLabel);
    furnaceInertBlockTemperatureLayout->addStretch(1);
    furnaceInertBlockTemperatureLayout->addWidget(diffTemperatureValueLabel);

    thermostatHeater = new HeaterSignalsView(tr("Термостат"));
    thermostatHeater->setValuePresision(3);

    QVBoxLayout * mainLayout = new QVBoxLayout;
    mainLayout->addLayout(sampleTemperatureLayout);
    mainLayout->addLayout(furnaceInertBlockTemperatureLayout);
    mainLayout->addWidget(thermostatHeater);
    setLayout(mainLayout);

    widgetRegulatorThermostat = new WidgetRegulatorFurnace(this);
    widgetRegulatorThermostat->setRegulator(Furnace::instance()->regulatorThermostat());

    connect(thermostatHeater,SIGNAL(statusLedClicked()),this,SLOT(showThermostatRegulatorSettingsWidget()));
}

void CalorimeterSignalsView::setValue(TerconData terconData)
{
    if (terconData.id == "{802cdd57-b870-4cdb-a1c2-2e430c70981c}"){
        thermostatHeater->setTemperature(terconData.value);
    }

    else if (terconData.id == "{41fada5a-879c-43ba-84d9-d565c0e03ead}"){
        if (resistanceLabel->isHidden()){
            resistanceLabel->show();
            resistanceValueLabel->show();
        }
        resistanceValueLabel->setText(tr("%1 Ом").arg(QString::number(terconData.value,'f',3)));
    }
    else if (terconData.id == "{ed87c4c3-ce64-4809-ba55-9c06f5f20271}"){
        if (diffTemperatureLabel->isHidden()){
            diffTemperatureLabel->show();
            diffTemperatureValueLabel->show();
        }
        diffTemperatureValueLabel->setText(tr("%1 мВ").arg(QString::number(terconData.value * 1000,'f',3)));
    }
}

void CalorimeterSignalsView::updateState(const json &state)
{
    if (QString::compare(state["sender"].get<std::string>().c_str(), "regulator-thermostat") == 0){
        thermostatHeater->updateState(state);
    }
}

void CalorimeterSignalsView::showThermostatRegulatorSettingsWidget()
{
    widgetRegulatorThermostat->setWindowTitle(tr("Регулятор термостата"));
    widgetRegulatorThermostat->exec();
}

