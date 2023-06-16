#include "dialogmeasuredistance.h"
#include "ui_dialogmeasuredistance.h"

DialogMeasureDistance::DialogMeasureDistance(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogMeasureDistance)
{
    ui->setupUi(this);

    connect(ui->toolButtonClose, &QToolButton::pressed, this, [&](){
        hide();
        points.clear();
        dists.clear();
        emit clearList();
    });
}

DialogMeasureDistance::~DialogMeasureDistance()
{
    delete ui;
}

void DialogMeasureDistance::addMeasurePoint(QPointF pt)
{
    float dist = 0;
    if (!points.isEmpty()) {
        dist = getDistanceBetweenCoordinates(pt, points.last()) + dists.last();
    }
    points.push_back(pt);
    dists.push_back(dist);

    calcTotalDistance();
}

void DialogMeasureDistance::calcTotalDistance()
{
    // Get total distance
    float totalDist = 0;
    foreach (auto dist, dists) {
        totalDist += dist;
    }

    ui->labelDistance->setText(getDitanceString(totalDist));
}

QString DialogMeasureDistance::getDitanceString(const float &dist)
{
    QString strDist;
    // Shorter than 10km
    if (dist < 10000) {
        strDist = QString("%1 m").arg(dist, 0, 'f', 0);
    }
    // Longer than 10 km
    else {
        strDist = QString("%1 km").arg(dist / 1000.0f, 0, 'f', 1, QLatin1Char('0'));
    }

    return strDist;
}

QList<float> DialogMeasureDistance::getDistanceList() const
{
    return dists;
}

bool DialogMeasureDistance::isEmpty()
{
    return points.isEmpty();
}

QList<QPointF> DialogMeasureDistance::getPointList() const
{
    return points;
}

#define RADIO_TERRESTRE 6372797.56085f
#define GRADOS_RADIANES (M_PI / 180.0f)
float DialogMeasureDistance::getDistanceBetweenCoordinates(QPointF coordSrc, QPointF coordDst)
{
    coordSrc.setY(coordSrc.y()  * GRADOS_RADIANES);
    coordSrc.setX(coordSrc.x() * GRADOS_RADIANES);
    coordDst.setY(coordDst.y()  * GRADOS_RADIANES);
    coordDst.setX(coordDst.x() * GRADOS_RADIANES);

    float haversine = (pow(sin((1.0f / 2) * (coordDst.y() - coordSrc.y())), 2))
                      + ((cos(coordSrc.y())) * (cos(coordDst.y())) * (pow(sin((1.0f / 2) * (coordDst.x() - coordSrc.x())), 2)));

    return RADIO_TERRESTRE * 2 * asin(fmin(1.0f, sqrt(haversine)));
}
