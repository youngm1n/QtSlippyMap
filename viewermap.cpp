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
    showGrid = false;

    connect(&mapTileLoader, &MapTileLoader::downloadedMapTile, this, &ViewerMap::downloadedMapTile);
}

void ViewerMap::setInitTileMap(const QString &url, const int newMaxZoom)
{
    if (mapTiles) {
        delete [] mapTiles;
    }

    maxZoom = newMaxZoom;
    mapTiles = new MAP_TILES[maxZoom + 1];

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

void ViewerMap::setShowGrid(bool newShowGrid)
{
    showGrid = newShowGrid;
    update();
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
    sizeScrLatLon = mapTileLoader.startDownloadTiles(centerLatLon, currentZoom, rect());
}

bool ViewerMap::convPixToCoord(const QPoint &pix, QPointF &coord)
{
    bool ret = false;

    for (MAP_TILES::iterator iter = mapTiles[currentZoom].begin(); iter != mapTiles[currentZoom].end(); ++iter) {
        if (iter.value().first.contains(pix)) {
            auto rectImg = iter.value().first;
            auto rectCoord = iter.value().second;
            coord.setX(rectCoord.left() + (pix.x() - rectImg.left()) / static_cast<float>(rectImg.width()) * rectCoord.width());
            coord.setY(rectCoord.top() - (pix.y() - rectImg.top()) / static_cast<float>(rectImg.height()) * abs(rectCoord.height()));
            ret = true;
            break;
        }
    }

    return ret;
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

void ViewerMap::downloadedMapTile(QString imgFilePath, QRect rectImg, QRectF rectCoord, int zoom)
{
    mapTiles[zoom].insert(imgFilePath, qMakePair(rectImg, rectCoord));

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

    p.fillRect(rect(), Qt::gray);

    // Draw chache map, during downloading
    if (mapTileLoader.isDownloading()) {
        p.drawImage(rectCacheMap, imgCacheMap);
    }
    // copy cache map, when download finished
    else {
        imgCacheMap = imgTempMap;
    }

    p.setPen(Qt::gray);
    // Draw map tiles
    for (MAP_TILES::iterator iter = mapTiles[currentZoom].begin(); iter != mapTiles[currentZoom].end(); ++iter) {
        p.drawImage(iter.value().first, QImage(iter.key()));

        // Draw grid
        if (showGrid) {
            p.drawRect(iter.value().first);
        }
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
            auto deltaScr = posMouse - dragMapStart;

            centerLatLon -= QPointF((deltaScr.x() / static_cast<float>(width())) * sizeScrLatLon.width(),
                          (-deltaScr.y() / static_cast<float>(height())) * sizeScrLatLon.height());

            if (centerLatLon.x() > 180) {
                centerLatLon.setX(centerLatLon.x() - 360);
            }
            if (centerLatLon.x() < -180) {
                centerLatLon.setX(centerLatLon.x() + 360);
            }
            if (centerLatLon.y() > 90) {
                centerLatLon.setY(centerLatLon.y() - 180);
            }
            if (centerLatLon.y() < -90) {
                centerLatLon.setY(centerLatLon.y() + 180);
            }

            dragMapStart = posMouse;
            rectCacheMap.setSize(rect().size());
            rectCacheMap.moveCenter(posMouse);
            updateMapTiles();
            update();
        }
        else {
            QPointF latLon;
            if (convPixToCoord(posMouse, latLon)) {
                mouseLatLon = latLon;
            }
            update();
        }

        return true;
    }
    return QOpenGLWidget::eventFilter(watched, event);
}

void ViewerMap::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
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
    Q_UNUSED(event);

    if (event->button() == Qt::LeftButton) {
        QPointF newCenterLatLon;
        if (convPixToCoord(event->pos(), newCenterLatLon)) {
            centerLatLon = newCenterLatLon;
            rectCacheMap.setSize(rect().size());
            rectCacheMap.moveCenter(event->pos());
            updateMapTiles();
            update();
        }
    }
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

        if (currentZoom < 3)            currentZoom = 3;
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

