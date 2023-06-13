#ifndef GLOBAL_H
#define GLOBAL_H

#define RadToDeg    ( 180 / M_PI)
#define DegToRad    ( M_PI / 180.0 )

#include <QString>

#include "formcoordinate.h"

enum COORD_TYPE { COORD_TYPE_DEG, COORD_TYPE_DMS };

void registerCoordinateWidget(FormCoordinate *w);
void setCoordType(int type);
QString getCoordString(float value);
void convDegToDms(float dec, int &deg, int &min, float &sec);
float convDmsToDeg(int deg, int min, float sec);

#endif // GLOBAL_H
