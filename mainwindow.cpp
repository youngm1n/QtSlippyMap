#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "global.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

//    showMaximized();

    // Coordinate type
    registerCoordinateWidget(ui->widgetCurrentLat);
    registerCoordinateWidget(ui->widgetCurrentLon);
    connect(ui->radioButtonDeg, &QRadioButton::toggled, this, [&](bool checked){
        setCoordType(checked ? COORD_TYPE_DEG : COORD_TYPE_DMS);
        ui->openGLWidgetMapView->update();
    });
    setCoordType(COORD_TYPE_DEG);

    // Init tile map server
    ui->openGLWidgetMapView->setUrlTileMap(ui->comboBoxTileMapUrl->currentText());
    connect(ui->comboBoxTileMapUrl, &QComboBox::currentTextChanged, ui->openGLWidgetMapView, &ViewerMap::setUrlTileMap);

    // Init current location
    ui->widgetCurrentLat->setValue(24.458510f);
    ui->widgetCurrentLon->setValue(54.397629f);
    connect(ui->openGLWidgetMapView, &ViewerMap::updateCurrentLocation, this, &MainWindow::updateCurrentLocation);
    connect(ui->pushButtonUpdateLocation, &QPushButton::pressed, this, [&]() {
        ui->openGLWidgetMapView->setCurrentLocation(ui->widgetCurrentLat->getValue(), ui->widgetCurrentLon->getValue());
    });
    ui->pushButtonUpdateLocation->pressed();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::updateCurrentLocation(float latitude, float longitude)
{
    ui->widgetCurrentLat->setValue(latitude);
    ui->widgetCurrentLon->setValue(longitude);
}
