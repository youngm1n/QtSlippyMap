#include "viewermap.h"

#include <QApplication>
#include <QWheelEvent>
#include <QPainter>

ViewerMap::ViewerMap(QWidget *parent) : QOpenGLWidget(parent)
{
    qApp->installEventFilter(this);
    zoom = 10;

    connect(&mapTileLoader, &MapTileLoader::downloadedMapTile, this, &ViewerMap::downloadedMapTile);
}

void ViewerMap::setUrlTileMap(const QString &newUrlTileMap)
{
    mapTileLoader.setUrlTileMap(newUrlTileMap);
}

void ViewerMap::resizeGL(int w, int h)
{
    updateMapTiles();
}

void ViewerMap::updateMapTiles()
{
//    float radius = (width() > height() ? width() : height()) / 2.0f;
//    radius = sqrt(radius * radius + radius * radius) * 2;
//    rectMapDraw.setSize(QSize(radius, radius));
    rectMapDraw = rect();

    mapTileLoader.getMapFromCoordinate(currentLat, currentLon, zoom, rectMapDraw);
}

//void ViewerMap::convScreenPosToLatLon(QPointF pos, double &lat, double &lon)
//{
//    double dist = 0, bearing = 0;
//    convScreenPosToLatLon(pos, lat, lon, dist, bearing);
//}

//void ViewerMap::convScreenPosToLatLon(QPointF pos, double &lat, double &lon, double &dist, double &bearing)
//{
//    dist = sqrt((pos.x() - rect().center().x()) * (pos.x() - rect().center().x()) + (pos.y() - rect().center().y()) * (pos.y() - rect().center().y())) * realDistPerScrPix;
//    bearing = atan2(pos.y() - rect().center().y(), pos.x() - rect().center().x()) + M_PI / 2;
//    if (bearing < 0) {
//        bearing += (M_PI * 2);
//    }

//    getDestinationLatLon(currentLat, currentLon, dist, bearing, lat, lon);
//}

//void ViewerMap::getDestinationLatLon(double srcLat, double srcLon, double distance, double bearing, double &dstLat, double &dstLon)
//{
//    double angDist = distance / 6371000.0;
//    srcLat *= DegToRad;
//    srcLon *= DegToRad;

//    dstLat = asin(sin(srcLat) * cos(angDist) + cos(srcLat) * sin(angDist) * cos(bearing));

//    double forAtana = sin(bearing) * sin(angDist) * cos(srcLat);
//    double forAtanb = cos(angDist) - sin(srcLat) * sin(dstLat);

//    dstLon = srcLon + atan2(forAtana, forAtanb);

//    dstLat *= RadToDeg;
//    dstLon *= RadToDeg;
//}

void ViewerMap::downloadedMapTile(QString imgFilePath, QRect rectScr)
{
    if (zoom < MAX_ZOOM_COUNT) {
        mapTiles[zoom].insert(imgFilePath, rectScr);
        update();
    }
}

void ViewerMap::setCurrentLocation(double newCurrentLat, double newCurrentLon)
{
    currentLat = newCurrentLat;
    currentLon = newCurrentLon;
}

bool ViewerMap::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == this) {

    }
    return QOpenGLWidget::eventFilter(watched, event);
}

void ViewerMap::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter p;
    p.begin(this);

    p.setPen(Qt::red);

    for (QMap<QString, QRect>::iterator iter = mapTiles[zoom].begin(); iter != mapTiles[zoom].end(); ++iter) {
        p.drawImage(iter.value(), QImage(iter.key()));
        p.drawRect(iter.value());
    }

    p.end();
}

void ViewerMap::mousePressEvent(QMouseEvent *event)
{

}

void ViewerMap::mouseReleaseEvent(QMouseEvent *event)
{

}

void ViewerMap::mouseDoubleClickEvent(QMouseEvent *event)
{

}

void ViewerMap::wheelEvent(QWheelEvent *event)
{
    if (event->angleDelta().y() > 0) {
        zoom++;
    }
    else {
        zoom--;
    }

    if (zoom < 0)       zoom = 0;
    else if (zoom > 19) zoom = 19;

    updateMapTiles();
    update();
}

