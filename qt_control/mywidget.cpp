#include "mywidget.h"
#include "ui_mywidget.h"

MyWidget::MyWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::MyWidget),mControlCam(std::make_shared<ControlCAM>())
{
    ui->setupUi(this);
    connect(ui->pbQuit,&QPushButton::clicked,qApp,&QApplication::quit);

    connect(ui->pbCamON,&QPushButton::clicked,[this]() {
        mControlCam->turnON();
    });

    connect(ui->pbCamOFF,&QPushButton::clicked,[this]() {
        mControlCam->turnOFF();
    });

    connect(mControlCam.get(),&ControlCAM::imageLoaded,[this](const QImage &qImg) {
        ui->mImage->setPixmap(QPixmap::fromImage(qImg));
        return;
    });

    connect(ui->mAutomatic,&QCheckBox::clicked,[this](bool checked){
        ui->mDial->setEnabled(!checked);
        mControlCam->enableSubscriptionServo(checked);
    });

    connect(ui->mDial,&QDial::valueChanged,[this](int value){
        if (value<90) {ui->mDial->setValue(90); return;}
        if (value>270) {ui->mDial->setValue(270);return;}
        if (!ui->mAutomatic->isChecked()) mControlCam->rotateServo(value-90);
    });


    QPixmap emptyPixmap(320, 240); // Crée un QPixmap vide avec les dimensions souhaitées
    emptyPixmap.fill(Qt::black);
    ui->mImage->setPixmap(emptyPixmap);

    connect(mControlCam.get(),&ControlCAM::newAngle,[this](int angle) {
        ui->mDial->setValue(angle+90);
    });
}

MyWidget::~MyWidget()
{
    delete ui;
}
