#include "viewermap.h"

#include <QApplication>
#include <QWheelEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QFontMetrics>

#include "global.h"

ViewerMap::ViewerMap(QWidget *parent) : QOpenGLWidget(parent)
{
    QSurfaceFormat f;
    f.setSamples(16);
    setFormat(f);


    qApp->installEventFilter(this);

    mapTiles = nullptr;
    currentZoom = previousZoom = 10;
    dragMap = false;
    showGridCenter = showGridTiles = false;

    currentMouseBtn = Qt::NoButton;

    // Set right click menu
    menu.setParent(this);
    menu.hide();
    auto action = new QAction("Measure Distance", &menu);
    menu.addAction(action);
    connect(action, &QAction::triggered, this, &ViewerMap::triggeredMenuAction);

    dlgMeas = new DialogMeasureDistance(this);
    dlgMeas->setWindowFlag(Qt::FramelessWindowHint);
    connect(dlgMeas, &DialogMeasureDistance::clearList, this, [&]() { setCursor(QCursor(Qt::CrossCursor)); });

    // connect image download function to maptileloader
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

void ViewerMap::setShowGridCenter(bool showGrid)
{
    showGridCenter = showGrid;
    update();
}

void ViewerMap::setShowGridTiles(bool showGrid)
{
    showGridTiles = showGrid;
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

// Convert screen pixel to coordinate (lat/lon)
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

// Convert coordinate to screen pixel
QPointF ViewerMap::convCoordToPix(const QPointF &coord)
{
    QPointF topLeft = centerLatLon + QPointF(-sizeScrLatLon.width() / 2.0f, sizeScrLatLon.height() / 2.0f);
    return QPointF(((coord.x() - topLeft.x()) / sizeScrLatLon.width()) * width(),
                  ((topLeft.y() - coord.y()) / sizeScrLatLon.height()) * height());
}

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

    // Start painting
    QPainter p(this);

    p.beginNativePainting();
    p.fillRect(rect(), Qt::gray);

    // Draw map tiles
    drawMapTiles(p);
    // Draw map tiles grid
    drawGridForMapTiles(p);
    // Draw center grid
    drawGridForCenter(p);
    // Draw Measure distance
    drawMeasurement(p);
    // Draw etc.
    drawInfomation(p);

    // End painting
    p.endNativePainting();
}

void ViewerMap::drawMapTiles(QPainter &p)
{
    // Draw chache map, during downloading
    if (mapTileLoader.isDownloading()) {
        p.drawImage(rectCacheMap, imgCacheMap);
    }
    // copy cache map, when download finished
    else {
        imgCacheMap = imgTempMap;
    }
    // Draw tiles
    for (MAP_TILES::iterator iter = mapTiles[currentZoom].begin(); iter != mapTiles[currentZoom].end(); ++iter) {
        p.drawImage(iter.value().first, QImage(iter.key()));
    }
}

void ViewerMap::drawGridForMapTiles(QPainter &p)
{
    if (showGridTiles) {
        p.setPen(QPen(Qt::gray, 1.0f, Qt::DotLine));
        for (MAP_TILES::iterator iter = mapTiles[currentZoom].begin(); iter != mapTiles[currentZoom].end(); ++iter) {
            auto rectTile = iter.value().first;
            auto rectGrid = rectTile;
            auto pos = iter.value().second;
            if (rectTile.top() <= 0 && rectTile.left() >= 0) {
                rectGrid.moveTop(0);
                p.drawText(rectGrid, Qt::AlignTop | Qt::AlignLeft, getCoordString(pos.left(), COORD_TYPE_LON));
                p.drawLine(rectGrid.left(), 0, rectGrid.left(), height());
            }
            if (rectTile.top() >= 0 && rectTile.left() <= 0 && rectTile.right() >= 0) {
                rectGrid.moveLeft(0);
                p.drawText(rectGrid, Qt::AlignTop | Qt::AlignLeft, getCoordString(pos.top(), COORD_TYPE_LAT));
                p.drawLine(0, rectGrid.top(), width(), rectGrid.top());
            }
        }
    }
}

void ViewerMap::drawGridForCenter(QPainter &p)
{
    if (showGridCenter) {
        p.setPen(QPen(Qt::red, 2, Qt::DashLine));
        p.drawLine(rect().center().x(), 0, rect().center().x(), height());
        p.drawText(rect().center().x(), 0, width(), height(), Qt::AlignTop, " " + getCoordString(centerLatLon.x(), COORD_TYPE_LON));
        p.drawLine(0, rect().center().y(), width(), rect().center().y());
        p.drawText(0, height() / 2, width(), height() / 2, Qt::AlignTop | Qt::AlignRight, getCoordString(centerLatLon.y(), COORD_TYPE_LAT) + " ");
    }
}

void ViewerMap::drawMeasurement(QPainter &p)
{
    if (dlgMeas->isVisible()) {
        if (!dlgMeas->isEmpty()) {
            p.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);

            // Draw Lines
            p.setPen(Qt::black);
            auto measPoints = dlgMeas->getPointList();
            auto startScrPt = convCoordToPix(measPoints.first());
            foreach (auto pt, measPoints) {
                auto curScrPt = convCoordToPix(pt);
                p.drawLine(startScrPt, curScrPt);
                startScrPt = curScrPt;
            }

            // Draw points & texts
            p.setPen(QPen(Qt::black, 2));
            p.setBrush(Qt::white);
            auto rtPt = QRectF(0, 0, 7, 7);
            auto rtText = QRectF(0, 0, 200, 50);
            auto measDists = dlgMeas->getDistanceList();
            for (int i = 0; i < measPoints.count(); i++) {
                // Draw point
                auto scrPt = convCoordToPix(measPoints.at(i));
                rtPt.moveCenter(scrPt);
                p.drawEllipse(rtPt);

                // Draw text
                if (i > 0) {
                    rtText.moveCenter(rtPt.center());
                    rtText.moveTop(rtPt.bottom());
                    p.translate(rtPt.center());
                    p.rotate(atan2f((scrPt.y() - startScrPt.y()), (scrPt.x() - startScrPt.x())) * RadToDeg);
                    p.translate(-rtPt.center());
                    p.drawText(rtText,
                               measDists.at(i) < 10000 ? QString("%1 m").arg(measDists.at(i), 0, 'f', 0) : QString("%1 km").arg(measDists.at(i) / 1000.0f, 0, 'f', 1),
                               Qt::AlignTop | Qt::AlignHCenter);
                    p.resetTransform();
                }

                startScrPt = scrPt;
            }
            p.setBrush(Qt::transparent);
            p.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing, false);
        }
    }
}

void ViewerMap::drawInfomation(QPainter &p)
{
    // Current mouse position
    p.setPen(Qt::black);
    auto strLatLon = QString(" %1, %2 ")
                         .arg(getCoordString(mouseLatLon.y(), COORD_TYPE_LAT), getCoordString(mouseLatLon.x(), COORD_TYPE_LON));
    auto sizeStrLatLon = QFontMetrics(p.font()).boundingRect(strLatLon).size();
    auto rectLatLon = QRect(QPoint(rect().bottomLeft()) - QPoint(0, sizeStrLatLon.height()), sizeStrLatLon);
    p.drawText(rectLatLon, strLatLon);
}

bool ViewerMap::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == this && event->type() == QEvent::MouseMove) {
        auto posMouse = static_cast<QMouseEvent *>(event)->pos();
        if (!dragMap && currentMouseBtn == Qt::LeftButton) {
            dragMap = true;
            dragMapStart = posMouse;
            setCursor(QCursor(Qt::ClosedHandCursor));
            menu.hide();
        }

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
    else if (event->type() == QEvent::Move) {
        QRect moveRect(QPoint(), dlgMeas->size());
        moveRect.moveCenter(rect().center() + QPoint(0, height() / 2 - moveRect.height()));
        dlgMeas->move(mapToGlobal(moveRect.topLeft()));
    }

    return QOpenGLWidget::eventFilter(watched, event);
}

void ViewerMap::mousePressEvent(QMouseEvent *event)
{
    currentMouseBtn = event->button();
    if (event->button() == Qt::RightButton && dlgMeas->isHidden()) {
        menu.popup(event->pos());
    }
}

void ViewerMap::mouseReleaseEvent(QMouseEvent *event)
{
    Q_UNUSED(event);

    currentMouseBtn = Qt::NoButton;

    // Start measure distance
    if (!dragMap && dlgMeas->isVisible()) {
        if (event->button() == Qt::LeftButton) {
            setCursor(QCursor(Qt::CrossCursor));

            addMeasurePoint(event->pos());
        }
    }

    // Cancel drag map
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

void ViewerMap::triggeredMenuAction(bool checked)
{
    Q_UNUSED(checked);
    auto action = reinterpret_cast<QAction *>(sender());
    if (action->text().contains("Measure Distance")) {
        addMeasurePoint(menu.geometry().topLeft());
        dlgMeas->show();
    }

    menu.hide();
}

void ViewerMap::addMeasurePoint(QPoint pos)
{
    QPointF latLon;
    if (convPixToCoord(pos, latLon)) {
        dlgMeas->addMeasurePoint(latLon);
    }
}
