#include "formcoordinate.h"
#include "ui_formcoordinate.h"

#include "global.h"

FormCoordinate::FormCoordinate(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FormCoordinate)
{
    ui->setupUi(this);
    connectItems(true);
}

FormCoordinate::~FormCoordinate()
{
    delete ui;
}


void FormCoordinate::setShowType(int newShowType)
{
    showType = newShowType;

    if (showType == COORD_SHOW_DEG) {
        ui->widgetDeg->show();
        ui->widgetDms->hide();
    }
    else {
        ui->widgetDeg->hide();
        ui->widgetDms->show();
    }
}

void FormCoordinate::setLatLonType(int newLatLonType)
{
    latLonType = newLatLonType;

    if (latLonType == COORD_TYPE_LAT) {
        ui->spinBoxDeg->setMaximum(90);
        ui->spinBoxDeg->setMinimum(-90);
        ui->doubleSpinBoxDeg->setMaximum(90);
        ui->doubleSpinBoxDeg->setMinimum(-90);
    }
    else {
        ui->spinBoxDeg->setMaximum(180);
        ui->spinBoxDeg->setMinimum(-180);
        ui->doubleSpinBoxDeg->setMaximum(180);
        ui->doubleSpinBoxDeg->setMinimum(-180);
    }
}

float FormCoordinate::getValue() const
{
    return value;
}

void FormCoordinate::setValue(float newValue)
{
    connectItems(false);

    value = newValue;
    ui->doubleSpinBoxDeg->setValue(value);

    int deg = 0, min = 0;
    float sec = 0;
    convDegToDms(value, deg, min, sec);

    ui->spinBoxDeg->setValue(deg);
    ui->spinBoxMin->setValue(min);
    ui->doubleSpinBoxSec->setValue(sec);

    connectItems(true);
}

void FormCoordinate::connectItems(bool con)
{
    if (con) {
        connect(ui->spinBoxDeg, SIGNAL(valueChanged(int)), this, SLOT(changeDmsValues(int)));
        connect(ui->spinBoxMin, SIGNAL(valueChanged(int)), this, SLOT(changeDmsValues(int)));
        connect(ui->doubleSpinBoxSec, SIGNAL(valueChanged(double)), this, SLOT(changeDmsValues(double)));
        connect(ui->doubleSpinBoxDeg, &QDoubleSpinBox::valueChanged, this, &FormCoordinate::changeDegValue);
    }
    else {
        disconnect(ui->spinBoxDeg, SIGNAL(valueChanged(int)), this, SLOT(changeDmsValues(int)));
        disconnect(ui->spinBoxMin, SIGNAL(valueChanged(int)), this, SLOT(changeDmsValues(int)));
        disconnect(ui->doubleSpinBoxSec, SIGNAL(valueChanged(double)), this, SLOT(changeDmsValues(double)));
        disconnect(ui->doubleSpinBoxDeg, &QDoubleSpinBox::valueChanged, this, &FormCoordinate::changeDegValue);
    }
}

void FormCoordinate::changeDmsValues(int arg)
{
    Q_UNUSED(arg);
    auto value = convDmsToDeg(ui->spinBoxDeg->value(),
                              ui->spinBoxMin->value(),
                              ui->doubleSpinBoxSec->value());
    ui->doubleSpinBoxDeg->setValue(value);
}

void FormCoordinate::changeDmsValues(double arg)
{
    changeDmsValues(static_cast<int>(arg));
}

void FormCoordinate::changeDegValue(double value)
{
    connectItems(false);

    int deg = 0, min = 0;
    float sec = 0;
    convDegToDms(value, deg, min, sec);

    ui->spinBoxDeg->setValue(deg);
    ui->spinBoxMin->setValue(min);
    ui->doubleSpinBoxSec->setValue(sec);

    connectItems(true);
}

