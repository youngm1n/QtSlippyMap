#include "global.h"

#include <QList>

QList<FormCoordinate *> coordWidgetList;
int coordType = COORD_SHOW_DEG;

void registerCoordinateWidget(FormCoordinate *w)
{
    coordWidgetList.push_back(w);
}

void setCoordType(int type)
{
    coordType = type;

    foreach (auto w, coordWidgetList) {
        w->setShowType(coordType);
    }
}

QString getCoordString(float value, int type)
{
    QString str;
    QString sign = value > 0 ? (type == COORD_TYPE_LAT ? "N" : "E") : (type == COORD_TYPE_LAT ? "S" : "W");
    if (coordType == COORD_SHOW_DEG) {
        str = sign + QString(" %1°").arg(abs(value), 0, 'f', 8, QLatin1Char('0'));
    }
    else {
        int deg, min;
        float sec;
        convDegToDms(value, deg, min, sec);
        str = sign + QString(" %1° %2\' %3\"").arg(abs(deg)).arg(min, 2, 10, QLatin1Char('0')).arg(sec, 0, 'f', 1, QLatin1Char('0'));
    }

    return str;
}

void convDegToDms(float dec, int &deg, int &min, float &sec)
{
    bool minus = dec < 0 ? true : false;
    dec = abs(dec);
    deg = static_cast<int>(dec * (minus ? -1 : 1));
    min = static_cast<int>((dec - deg) * 60);
    sec = ((dec - deg) * 60 - min) * 60;
}

float convDmsToDeg(int deg, int min, float sec)
{
    return static_cast<double>(deg) + min / 60.0f + sec / 3600.0f;
}
