#include "viewermap.h"

#include <QApplication>
#include <QWheelEvent>
#include <QMouseEvent>
#include <QPainter>

ViewerMap::ViewerMap(QWidget *parent) : QOpenGLWidget(parent)
{
    qApp->installEventFilter(this);
    currentZoom = previousZoom = 10;
    dragMap = false;

    connect(&mapTileLoader, &MapTileLoader::downloadedMapTile, this, &ViewerMap::downloadedMapTile);
}

void ViewerMap::setUrlTileMap(const QString &newUrlTileMap)
{
    mapTileLoader.setUrlTileMap(newUrlTileMap);
}

void ViewerMap::resizeGL(int w, int h)
{
    Q_UNUSED(w);
    Q_UNUSED(h);
    updateMapTiles();
}

void ViewerMap::updateMapTiles()
{
    mapTiles[currentZoom].clear();
    rectCurrentLatLon = mapTileLoader.startDownloadTiles(currentLat, currentLon, currentZoom, rect());
    qDebug() << rectCurrentLatLon;
}

QPointF ViewerMap::convPixToCoord(QPoint pt)
{
    QPointF coord((pt.x() / static_cast<float>(rect().width())) * rectCurrentLatLon.width(),
                  (pt.y() / static_cast<float>(rect().height())) * rectCurrentLatLon.height());

    return coord;
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

void ViewerMap::downloadedMapTile(QString imgFilePath, QRect rectScr, int zoom)
{
    mapTiles[zoom].insert(imgFilePath, rectScr);
    update();
}

void ViewerMap::setCurrentLocation(double newCurrentLat, double newCurrentLon)
{
    currentLat = newCurrentLat;
    currentLon = newCurrentLon;
}

void ViewerMap::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter p;
    p.begin(this);

    for (QMap<QString, QRect>::iterator iter = mapTiles[currentZoom].begin(); iter != mapTiles[currentZoom].end(); ++iter) {
        p.drawImage(iter.value(), QImage(iter.key()));
    }

    p.end();
}

bool ViewerMap::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == this && event->type() == QEvent::MouseMove) {
        auto posMouse = static_cast<QMouseEvent *>(event)->pos();
        if (dragMap) {
            auto dragDelta = posMouse - dragMapStart;
            auto dragCoord = convPixToCoord(dragDelta);
            currentLon -= dragCoord.x();
            currentLat -= dragCoord.y();
            updateMapTiles();
            update();

            dragMapStart = posMouse;
        }
        else {
            auto coordMouse = convPixToCoord(posMouse) + rectCurrentLatLon.topLeft();
            qDebug() << coordMouse;
        }

        return true;
    }
    return QOpenGLWidget::eventFilter(watched, event);
}

void ViewerMap::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::RightButton) {
        dragMap = true;
        dragMapStart = event->pos();
    }
}

void ViewerMap::mouseReleaseEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
    if (dragMap) {
        dragMap = false;
        emit updateCurrentLocation(currentLat, currentLon);
    }
}

void ViewerMap::mouseDoubleClickEvent(QMouseEvent *event)
{

}

void ViewerMap::wheelEvent(QWheelEvent *event)
{
    previousZoom = currentZoom;
    if (event->angleDelta().y() > 0) {
        currentZoom++;
    }
    else {
        currentZoom--;
    }

    if (currentZoom < 4)       currentZoom = 4;
    else if (currentZoom > 19) currentZoom = 19;

    if (previousZoom != currentZoom) {
        updateMapTiles();
        update();
    }
}

