#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "global.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

//    showMaximized();

    // Set tile map server
    ui->comboBoxTileMapUrl->addItem("https://tile.openstreetmap.org", 19);  // UserData = max zoom level
    ui->comboBoxTileMapUrl->addItem("https://a.tile.openstreetmap.org", 19);
    ui->comboBoxTileMapUrl->addItem("https://b.tile.openstreetmap.org", 19);
    ui->comboBoxTileMapUrl->addItem("https://c.tile.openstreetmap.org", 19);
    ui->comboBoxTileMapUrl->addItem("https://d.tile.openstreetmap.org", 19);
    ui->comboBoxTileMapUrl->addItem("https://tile.thunderforest.com/cycle", 22);
    ui->comboBoxTileMapUrl->setCurrentIndex(0);

    // Coordinate type
    registerCoordinateWidget(ui->widgetCurrentLat);
    registerCoordinateWidget(ui->widgetCurrentLon);
    connect(ui->radioButtonDeg, &QRadioButton::toggled, this, [&](bool checked){
        setCoordType(checked ? COORD_TYPE_DEG : COORD_TYPE_DMS);
        ui->openGLWidgetMapView->update();
    });
    setCoordType(COORD_TYPE_DEG);

    // Init tile map server
    ui->openGLWidgetMapView->setInitTileMap(ui->comboBoxTileMapUrl->currentText(),
                                            ui->comboBoxTileMapUrl->itemData(ui->comboBoxTileMapUrl->currentIndex()).toInt());
    connect(ui->comboBoxTileMapUrl, &QComboBox::currentIndexChanged, this, [&](int index) {
        ui->openGLWidgetMapView->setInitTileMap(ui->comboBoxTileMapUrl->currentText(),
                                                ui->comboBoxTileMapUrl->itemData(index).toInt());
    });

    // Init current location
    ui->widgetCurrentLat->setValue(24.458510f);
    ui->widgetCurrentLon->setValue(54.397629f);
    connect(ui->openGLWidgetMapView, &ViewerMap::updateCurrentLocation, this, &MainWindow::updateCurrentLocation);
    connect(ui->pushButtonUpdateLocation, &QPushButton::pressed, this, [&]() {
        ui->openGLWidgetMapView->setCurrentLocation(ui->widgetCurrentLat->getValue(), ui->widgetCurrentLon->getValue());
    });
    emit ui->pushButtonUpdateLocation->pressed();
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
