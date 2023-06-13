#include "formcoordinate.h"
#include "ui_formcoordinate.h"

#include "global.h"

FormCoordinate::FormCoordinate(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FormCoordinate)
{
    ui->setupUi(this);

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
        qDebug() << "Deg: " << value;
    });
}

FormCoordinate::~FormCoordinate()
{
    delete ui;
}

void FormCoordinate::setType(int newType)
{
    type = newType;

    if (type == COORD_TYPE_DEG) {
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

float FormCoordinate::getValue() const
{
    return value;
}

void FormCoordinate::setValue(float newValue)
{
    value = newValue;
    ui->doubleSpinBoxDeg->setValue(value);

    int deg = 0, min = 0;
    float sec = 0;
    convDegToDms(value, deg, min, sec);

    ui->spinBoxDeg->setValue(deg);
    ui->spinBoxMin->setValue(min);
    ui->doubleSpinBoxSec->setValue(sec);
}

void FormCoordinate::changeDmsValues()
{
    auto value = convDmsToDeg(ui->spinBoxDeg->value(),
                              ui->spinBoxMin->value(),
                              ui->doubleSpinBoxSec->value());
    ui->doubleSpinBoxDeg->setValue(value);
}
