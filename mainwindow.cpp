#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

//    showMaximized();

    // Init tile map server
    ui->openGLWidgetMapView->setUrlTileMap(ui->comboBoxTileMapUrl->currentText());
    connect(ui->comboBoxTileMapUrl, &QComboBox::currentTextChanged, ui->openGLWidgetMapView, &ViewerMap::setUrlTileMap);

    // Init current location
    connect(ui->pushButtonUpdateLocation, &QPushButton::pressed, this, [this]() {
        ui->openGLWidgetMapView->setCurrentLocation(ui->lineEditCurrentLat->text().toDouble(),
                                                    ui->lineEditCurrentLon->text().toDouble());
    });
    ui->pushButtonUpdateLocation->pressed();
}

MainWindow::~MainWindow()
{
    delete ui;
}
