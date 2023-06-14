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
        ui->labelMin->hide();
        ui->labelSec->hide();

        ui->spinBoxDeg->hide();
        ui->spinBoxMin->hide();
        ui->doubleSpinBoxSec->hide();

        ui->doubleSpinBoxDeg->show();
    }
    else {
        ui->doubleSpinBoxDeg->hide();

        ui->labelMin->show();
        ui->labelSec->show();

        ui->spinBoxDeg->show();
        ui->spinBoxMin->show();
        ui->doubleSpinBoxSec->show();
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
        connect(ui->spinBoxDeg, &QSpinBox::valueChanged, this, [&](int) { changeDmsValues(); });
        connect(ui->spinBoxMin, &QSpinBox::valueChanged, this, [&](int) { changeDmsValues(); });
        connect(ui->doubleSpinBoxSec, &QDoubleSpinBox::valueChanged, this, [&](double) { changeDmsValues(); });
        connect(ui->doubleSpinBoxDeg, &QDoubleSpinBox::valueChanged, this, [&](double value) {
            int deg = 0, min = 0;
            float sec = 0;
            convDegToDms(value, deg, min, sec);

            ui->spinBoxDeg->setValue(deg);
            ui->spinBoxMin->setValue(min);
            ui->doubleSpinBoxSec->setValue(sec);
        });
    }
}

void FormCoordinate::changeDmsValues()
{
    auto value = convDmsToDeg(ui->spinBoxDeg->value(),
                              ui->spinBoxMin->value(),
                              ui->doubleSpinBoxSec->value());
    ui->doubleSpinBoxDeg->setValue(value);
}
