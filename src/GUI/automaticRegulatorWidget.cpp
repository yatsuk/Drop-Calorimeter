#include "automaticRegulatorWidget.h"
#include <QFont>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QMessageBox>
#include <QFileDialog>
#include <QTime>
#include <QDebug>

AutomaticRegulatorWidget::AutomaticRegulatorWidget(QWidget *parent) :
    QGroupBox(parent)
{
    regulatorOn = false;
    temperatureSegments = 0;

    QFont captionFont;
    captionFont.setBold(true);

    setTitle(tr("Автоматическое регулирование"));

    enableAutoRegulator = new QRadioButton(tr("Включить"));
    enableAutoRegulator->setFont(captionFont);

    tableSegments = new QTableWidget(0,4);
    tableSegments->setEditTriggers(QAbstractItemView::NoEditTriggers);
    for (int i =0; i<tableSegments->columnCount();++i)
        tableSegments->setColumnWidth(i,70);

    QStringList headerSegmentsTable;
    headerSegmentsTable << tr("Тип") << tr("%1C").arg(QChar(176))<< tr("К/мин") << tr("Время");
    tableSegments->setHorizontalHeaderLabels(headerSegmentsTable);

    autoRegulatorLabel = new QLabel(tr("Таблица температурных сегментов"));
    totalDurationLabel = new QLabel(tr("Общая продолжительность"));
    remainingTimeLabel = new QLabel(tr("Оставшееся время"));

    addSegmentButton = new QPushButton(tr("Добавить"));
    addSegmentButton->setToolTip(tr("Добавить сегмент к температурной программе."));
    deleteSegmentButton = new QPushButton(tr("Удалить"));
    deleteSegmentButton->setToolTip(tr("Удалить выделенный сегмент из температурной программы."));
    modifSegmentButton = new QPushButton(tr("Изменить"));
    modifSegmentButton->setToolTip(tr("Изменть выделенный сегмент температурной программы."));
    saveProgrammButton = new QPushButton(tr("Сохранить"));
    saveProgrammButton->setToolTip(tr("Сохранить температурную программу в файл."));
    loadProgrammButton = new QPushButton(tr("Загрузить"));
    loadProgrammButton->setToolTip(tr("Загрузить температурную программу из файла."));
    stopCurrentTemperature = new QPushButton(tr("Стоянка"));
    stopCurrentTemperature->setCheckable(true);
    stopCurrentTemperature->setToolTip(tr("Уставка регулятора равняется текущей температуре."));

    goToSelectedSegmentButton = new QPushButton(tr("Перейти к сегменту"));
    goToSelectedSegmentButton->setToolTip(tr("Переход регулятора к выделенному сегменту температурной программы."));

    autoRegulatorOperationLabel =  new QLabel(tr("Действия над температурными сегментами."));
    QGroupBox * boxOperSegmentsButton = new QGroupBox();
    QGridLayout * boxButtonGridLayout = new QGridLayout;
    boxButtonGridLayout->addWidget(addSegmentButton,0,0);
    boxButtonGridLayout->addWidget(modifSegmentButton,0,1);
    boxButtonGridLayout->addWidget(deleteSegmentButton,0,2);
    boxButtonGridLayout->addWidget(saveProgrammButton,1,0);
    boxButtonGridLayout->addWidget(loadProgrammButton,1,1);
    boxButtonGridLayout->addWidget(stopCurrentTemperature,1,2);
    boxOperSegmentsButton->setLayout(boxButtonGridLayout);

    QVBoxLayout * autoRegulatorVLayout = new QVBoxLayout;
    autoRegulatorVLayout->addWidget(enableAutoRegulator);
    autoRegulatorVLayout->addWidget(autoRegulatorLabel,0,Qt::AlignCenter);
    autoRegulatorVLayout->addWidget(tableSegments);
    autoRegulatorVLayout->addWidget(totalDurationLabel);
    //autoRegulatorVLayout->addWidget(remainingTimeLabel);
    autoRegulatorVLayout->addWidget(autoRegulatorOperationLabel,0,Qt::AlignCenter);
    autoRegulatorVLayout->addWidget(boxOperSegmentsButton);
    autoRegulatorVLayout->addWidget(goToSelectedSegmentButton);

    setLayout(autoRegulatorVLayout);

    addSegmentDialog = new AddTempSegmentDialog(this);
    addSegmentDialog->setModal(true);

    connect (enableAutoRegulator,SIGNAL(pressed()),this,SLOT(autoModeOn()));
    connect (enableAutoRegulator,SIGNAL(clicked()),this,SLOT(checkBoxReleased()));
    connect (addSegmentButton,SIGNAL(clicked()),this,SLOT(addSegment()));
    connect (deleteSegmentButton,SIGNAL(clicked()),this,SLOT(deleteSegment()));
    connect (saveProgrammButton,SIGNAL(clicked()),this,SLOT(saveTemperatureProgramm()));
    connect (loadProgrammButton,SIGNAL(clicked()),this,SLOT(loadTemperatureProgramm()));
    connect (modifSegmentButton,SIGNAL(clicked()),this,SLOT(editTemperatureProgramm()));
    connect (goToSelectedSegmentButton,SIGNAL(clicked()),this,SLOT(goToSegmentClicked()));
    connect (stopCurrentTemperature,SIGNAL(toggled(bool)),this,SIGNAL(regCurrentTemperature(bool)));
}

void AutomaticRegulatorWidget::setTemperatureProgramm(Segments * tProgramm){
    temperatureSegments = tProgramm;
    connect (temperatureSegments,SIGNAL(update()),this,SLOT(updateTable()));
    connect (temperatureSegments,SIGNAL(totalDuration(double)),this,SLOT(setTotalDuration(double)));
    connect (temperatureSegments,SIGNAL(remainingTime(double)),this,SLOT(setRemainingTime(double)));

    connect (addSegmentDialog,SIGNAL(newSegment(Segment*)),
             temperatureSegments,SLOT(addSegment(Segment*)));

}

void AutomaticRegulatorWidget::addSegment(){
    addSegmentDialog->show();
}

void AutomaticRegulatorWidget::editTemperatureProgramm(){
    AddTempSegmentDialog * editDialog= new AddTempSegmentDialog(this);
    editDialog->setAttribute(Qt::WA_DeleteOnClose);
    editDialog->setModal(true);
    editDialog->setWindowTitle(tr("Изменение сегмента"));
    editDialog->show();
}

void AutomaticRegulatorWidget::loadTemperatureProgramm(){
    if(!temperatureSegments->isEmpty()){
        int ret = QMessageBox::warning(this,
                                       tr("Загрузка температурной программы."),
                                       tr("Подтвердите загрузку температурной программы.\nТекущая температурная программа будет удалена."),
                                       QMessageBox::Yes|QMessageBox::No,
                                       QMessageBox::No
                                       );

        if (ret==QMessageBox::No)
            return;
    }
    temperatureSegments->load(QFileDialog::getOpenFileName(this));
}

void AutomaticRegulatorWidget::goToSegmentClicked(){
    int currentRow = tableSegments->currentRow();
    if (currentRow>0){
        emit goToSegment(currentRow);
    }
}

void AutomaticRegulatorWidget::saveTemperatureProgramm(){
    if(!temperatureSegments->isEmpty())
        temperatureSegments->save(QFileDialog::getSaveFileName(this));
}

void AutomaticRegulatorWidget::deleteSegment(){
    int currentRow = tableSegments->currentRow();
    if (currentRow>=0){
        int ret = QMessageBox::warning(this,
                                       tr("Удаление сегмента."),
                                       tr("Подтвердите удаление сегмента."),
                                       QMessageBox::Yes|QMessageBox::No,
                                       QMessageBox::No
                                       );

        if (ret==QMessageBox::Yes)
            temperatureSegments->deleteSegment(tableSegments->currentRow());
    }
}

void AutomaticRegulatorWidget::setCurrentTemperatureSegment(int segmentNumber){
    updateTable();

    for (int i =0;i <tableSegments->columnCount();++i){
        QTableWidgetItem * item = tableSegments->item(segmentNumber,i);
        if(item)
            item->setBackgroundColor(Qt::cyan);
    }
}

void AutomaticRegulatorWidget::setTotalDuration(double duration){
    totalDurationLabel->setText(tr("Общая продолжительность ")+QTime(0,0).addSecs(duration*60).toString("hh:mm:ss"));
}

void AutomaticRegulatorWidget::setRemainingTime(double time){
    remainingTimeLabel->setText(tr("Оставшееся время ")+QTime(0,0).addSecs(time*60).toString("hh:mm:ss"));
}

void AutomaticRegulatorWidget::updateTable(){
    tableSegments->clear();
    tableSegments->setRowCount(temperatureSegments->segmentCount());

    for (int i =0; i<tableSegments->columnCount();++i)
        tableSegments->setColumnWidth(i,70);

    QStringList headerSegmentsTable;
    headerSegmentsTable << tr("Тип") << tr("%1C").arg(QChar(176))<< tr("К/мин") << tr("Время");
    tableSegments->setHorizontalHeaderLabels(headerSegmentsTable);

    for (int i=0;i<temperatureSegments->segmentCount();++i){
        QTableWidgetItem * typeSegment = new QTableWidgetItem(temperatureSegments->segment(i)->typeTitle());
        typeSegment->setTextAlignment(Qt::AlignCenter);
        QTableWidgetItem * velocity = new QTableWidgetItem(QString::number(temperatureSegments->segment(i)->velocity()));
        velocity->setTextAlignment(Qt::AlignCenter);
        QTableWidgetItem * endTemperature = new QTableWidgetItem(QString::number(temperatureSegments->segment(i)->endTemperature()));
        endTemperature->setTextAlignment(Qt::AlignCenter);
        QTableWidgetItem * duration = new QTableWidgetItem(QTime(0,0).addSecs(temperatureSegments->segment(i)->duration()*60).toString("hh:mm:ss"));
        duration->setTextAlignment(Qt::AlignCenter);

        tableSegments->setItem(i,0,typeSegment);
        tableSegments->setItem(i,1,endTemperature);
        tableSegments->setItem(i,2,velocity);
        tableSegments->setItem(i,3,duration);
    }
}

void AutomaticRegulatorWidget::checkBoxReleased(){
    enableAutoRegulator->setChecked(true);
}

void AutomaticRegulatorWidget::setRegulatorOn(bool on){
    regulatorOn = on;
}

void AutomaticRegulatorWidget::autoModeOn(){
    if(!enableAutoRegulator->isChecked()){
        int ret = QMessageBox::Yes;
        if(regulatorOn)
            ret = QMessageBox::warning(this,
                                       tr("Переключение регулятора в автоматический режим."),
                                       tr("Подтвердите переключение регулятора в автоматический режим."),
                                       QMessageBox::Yes|QMessageBox::No,
                                       QMessageBox::No
                                       );

        if (ret==QMessageBox::Yes){
            setEnabledWidget(true);
            emit autoRegulatorEnabled();
        }
    }
}

void AutomaticRegulatorWidget::setEnabledWidget(bool enabled){
    autoRegulatorLabel->setEnabled(enabled);
    autoRegulatorOperationLabel->setEnabled(enabled);
    totalDurationLabel->setEnabled(enabled);
    remainingTimeLabel->setEnabled(enabled);
    enableAutoRegulator->setChecked(enabled);
    tableSegments->setEnabled(enabled);
    addSegmentButton->setEnabled(enabled);
    deleteSegmentButton->setEnabled(enabled);
    modifSegmentButton->setEnabled(false);
    goToSelectedSegmentButton->setEnabled(enabled);
    saveProgrammButton->setEnabled(enabled);
    loadProgrammButton->setEnabled(enabled);
    stopCurrentTemperature->setEnabled(enabled);
}

bool AutomaticRegulatorWidget::isEnabledWidget(){
    return enableAutoRegulator->isChecked();
}
