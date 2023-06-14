#include "viewermap.h"

#include <QApplication>
#include <QWheelEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QFontMetrics>

#include "global.h"

ViewerMap::ViewerMap(QWidget *parent) : QOpenGLWidget(parent)
{
    qApp->installEventFilter(this);

    mapTiles = nullptr;
    currentZoom = previousZoom = 10;
    dragMap = false;

    connect(&mapTileLoader, &MapTileLoader::downloadedMapTile, this, &ViewerMap::downloadedMapTile);
}

void ViewerMap::setInitTileMap(const QString &url, const int newMaxZoom)
{
    if (mapTiles) {
        delete [] mapTiles;
    }

    maxZoom = newMaxZoom;
    mapTiles = new QMap<QString, QRect>[maxZoom + 1];

    mapTileLoader.setUrlTileMap(url);
    updateMapTiles();
    update();

    qDebug() << "Server changed: " << url;
}

void ViewerMap::setCurrentLocation(float newCurrentLat, float newCurrentLon)
{
    centerLatLon.setX(newCurrentLon);
    centerLatLon.setY(newCurrentLat);
}

void ViewerMap::resizeGL(int w, int h)
{
    rectCacheMap = QRect(0, 0, w, h);
    imgTempMap = QImage(QSize(w, h), QImage::Format_RGB32);
    updateMapTiles();
}

void ViewerMap::updateMapTiles()
{
    mapTiles[currentZoom].clear();
    rectCurrentLatLon = mapTileLoader.startDownloadTiles(centerLatLon, currentZoom, rect());
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

void ViewerMap::downloadedMapTile(QString imgFilePath, QRect rectImg, int zoom)
{
    mapTiles[zoom].insert(imgFilePath, rectImg);

    // Save cache image
    if (mapTileLoader.isDownloading()) {
        QPainter p(&imgTempMap);
        p.beginNativePainting();
        p.drawImage(rectImg, QImage(imgFilePath));
        p.endNativePainting();
    }

    update();
}

void ViewerMap::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter p(this);
    p.beginNativePainting();

    // Draw map tiles
    p.fillRect(rect(), Qt::black);

    if (mapTileLoader.isDownloading()) {
        p.drawImage(rectCacheMap, imgCacheMap);
    }
    else {
        imgCacheMap = imgTempMap;
        qDebug() << "Save";
    }

    for (QMap<QString, QRect>::iterator iter = mapTiles[currentZoom].begin(); iter != mapTiles[currentZoom].end(); ++iter) {
        p.drawImage(iter.value(), QImage(iter.key()));
    }

    // Current mouse position
    p.setPen(Qt::black);
    auto strLatLon = QString(" %1, %2 ").arg(getCoordString(mouseLatLon.y()), getCoordString(mouseLatLon.x()));
    auto sizeStrLatLon = QFontMetrics(p.font()).boundingRect(strLatLon).size();
    auto rectLatLon = QRect(QPoint(rect().bottomLeft()) - QPoint(0, sizeStrLatLon.height()), sizeStrLatLon);
    p.drawText(rectLatLon, strLatLon);

    p.endNativePainting();
}

bool ViewerMap::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == this && event->type() == QEvent::MouseMove) {
        auto posMouse = static_cast<QMouseEvent *>(event)->pos();
        if (dragMap) {
            auto dragDelta = posMouse - dragMapStart;
            auto dragCoord = convPixToCoord(dragDelta);
            centerLatLon -= dragCoord;
            rectCacheMap.setSize(rect().size());
            rectCacheMap.moveCenter(posMouse);
            updateMapTiles();
            update();

            dragMapStart = posMouse;
        }
        else {
            mouseLatLon = convPixToCoord(posMouse) + rectCurrentLatLon.topLeft();
//            update();
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

        setCursor(QCursor(Qt::ClosedHandCursor));
    }

    update();
}

void ViewerMap::mouseReleaseEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
    if (dragMap) {
        dragMap = false;
        emit updateCurrentLocation(centerLatLon.y(), centerLatLon.x());

        setCursor(QCursor(Qt::ArrowCursor));
    }

    update();
}

void ViewerMap::mouseDoubleClickEvent(QMouseEvent *event)
{

}

void ViewerMap::wheelEvent(QWheelEvent *event)
{
    if (!mapTileLoader.isDownloading()) {
        previousZoom = currentZoom;
        if (event->angleDelta().y() > 0) {
            currentZoom++;
        }
        else {
            currentZoom--;
        }

        if (currentZoom < 4)            currentZoom = 4;
        else if (currentZoom > maxZoom) currentZoom = maxZoom;

        if (previousZoom != currentZoom) {
            if (currentZoom > previousZoom) {
                rectCacheMap.setSize(rect().size() * 2);
            }
            else {
                rectCacheMap.setSize(rect().size() / 2);
            }
            rectCacheMap.moveCenter(rect().center());

            updateMapTiles();
            update();
        }
    }
}

