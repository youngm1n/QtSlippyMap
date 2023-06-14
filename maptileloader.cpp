#include "maptileloader.h"

#include <QDir>
#include <QFile>

#include "global.h"

#define MAP_LOCAL_ROOT      "./Map"
#define MAP_TILE_PIX_SIZE   256

MapTileLoader::MapTileLoader(QObject *parent)
    : QObject{parent}
{
    QDir(MAP_LOCAL_ROOT).removeRecursively();
    QDir().mkdir(MAP_LOCAL_ROOT);
    for (int zoom = 0; zoom < 20; zoom++) {
        mapLonSizePerZoom.insert(zoom, 360.0 / pow(2, zoom));
    }

    for (int i = 0; i < MAX_NAM_COUNT; i++) {
        connect(&timerDownloadQue[i], &QTimer::timeout, this, &MapTileLoader::timeoutDownloadQueue);
        timerDownloadQue[i].setProperty("NO", i);
        timerDownloadQue[i].start(1);
    }

    totalDownloadCount = curDownloadedCount = 0;
}

void MapTileLoader::setUrlTileMap(const QString &newUrlTileMap)
{
    QDir(MAP_LOCAL_ROOT).removeRecursively();
    QDir().mkdir(MAP_LOCAL_ROOT);

    // Set tile map server base
    urlTileMap = newUrlTileMap;
}

void MapTileLoader::resetDownloadQueue()
{
    queDownloadReq.clear();
}

QSizeF MapTileLoader::startDownloadTiles(QPointF center, int zoom, QRect rect)
{
    auto maxTileCount = static_cast<int>(pow(2, zoom));
    auto tileNoCenter = QPoint(longitudeToTileX(center.x(), zoom), latitudeToTileY(center.y(), zoom));
    auto tileNoCount = QPoint(ceil(rect.width() / 2.0 / static_cast<double>(MAP_TILE_PIX_SIZE)) + 1,
                              ceil(rect.height() / 2.0 / static_cast<double>(MAP_TILE_PIX_SIZE)) + 1);
    auto tileNoStart = tileNoCenter - tileNoCount;
    auto tileNoEnd = tileNoCenter + tileNoCount;

    auto tileRealCenter = QRectF(QPointF(tileXtoLongitude(tileNoCenter.x(), zoom), tileYtoLatitude(tileNoCenter.y(), zoom)),
                                 QPointF(tileXtoLongitude(tileNoCenter.x() + 1, zoom), tileYtoLatitude(tileNoCenter.y() + 1, zoom)));

    auto scrDelta = QPoint((center.x() - tileRealCenter.x()) / tileRealCenter.width() * MAP_TILE_PIX_SIZE,
                           (center.y() - tileRealCenter.y()) / tileRealCenter.height() * MAP_TILE_PIX_SIZE);
    auto scrStart = QPoint(rect.center() - QPoint(tileNoCount.x() * MAP_TILE_PIX_SIZE, tileNoCount.y() * MAP_TILE_PIX_SIZE)) - scrDelta;

    // Init downloading check parameters
    totalDownloadCount = tileNoCount.x() * tileNoCount.y() * 4;
    curDownloadedCount = 0;

    // Reset queue and start getting tiles
    resetDownloadQueue();

    QPointF startScrLatLon, endScrLatLon;
    auto scrY = scrStart.y();
    for (int y = tileNoStart.y(); y < tileNoEnd.y(); y++) {
        auto scrX = scrStart.x();
        for (int x = tileNoStart.x(); x < tileNoEnd.x(); x++) {
            QRect imgRect = QRect(scrX, scrY, MAP_TILE_PIX_SIZE, MAP_TILE_PIX_SIZE);
            getMapTile(zoom, rescaleTileIndex(x, maxTileCount), rescaleTileIndex(y, maxTileCount), imgRect);

            if (imgRect.contains(rect.topLeft())) {
                startScrLatLon = convPixToCoord(zoom, x, y, imgRect, rect.topLeft());
            }
            if (imgRect.contains(rect.bottomRight())) {
                endScrLatLon = convPixToCoord(zoom, x, y, imgRect, rect.bottomRight());
            }

            scrX += MAP_TILE_PIX_SIZE;
        }
        scrY += MAP_TILE_PIX_SIZE;
    }

    // return the screen's real coordinate size
    return QSizeF(abs(endScrLatLon.x() - startScrLatLon.x()), abs(endScrLatLon.y() - startScrLatLon.y()));
}

QPointF MapTileLoader::convPixToCoord(const int &zoom, const int &x, const int &y, QRect scrRect, QPoint pix)
{
    auto lonLeft = tileXtoLongitude(x, zoom);
    auto latTop = tileYtoLatitude(y, zoom);
    auto lonRight = tileXtoLongitude(x + 1, zoom);
    auto latBottom = tileYtoLatitude(y + 1, zoom);

    auto lon = lonLeft + (pix.x() - scrRect.left()) / static_cast<float>(scrRect.width()) * (lonRight - lonLeft);
    auto lat = latTop - (pix.y() - scrRect.top()) / static_cast<float>(scrRect.height()) * (latTop - latBottom);

    return QPointF(lon, lat);
}

// Rescale a tile index, when the index is over the range
int MapTileLoader::rescaleTileIndex(const int &no, const int &maxCount)
{
    int newNo = no;
    int newMaxCount = maxCount;
    if (newMaxCount == 0) {
        newMaxCount = 1;
    }

    if (newNo < 0) {
        newNo = newNo % newMaxCount;
        if (newNo) {
            newNo += newMaxCount;
        }
    }
    if (newNo >= maxCount) {
        newNo = newNo % newMaxCount;
    }

    return newNo;
}

bool MapTileLoader::isDownloading()
{
    return totalDownloadCount > curDownloadedCount;
}

int MapTileLoader::longitudeToTileX(double lon, int zoom)
{
    return static_cast<int>(floor((lon + 180.0) / 360.0 * (1 << zoom)));
}

int MapTileLoader::latitudeToTileY(double lat, int zoom)
{
    double latRad = lat * DegToRad;
    return static_cast<int>(floor((1.0 - asinh(tan(latRad)) / M_PI) / 2.0 * (1 << zoom)));
}

double MapTileLoader::tileXtoLongitude(int x, int zoom)
{
    return x / static_cast<double>(1 << zoom) * 360.0 - 180.0;
}

double MapTileLoader::tileYtoLatitude(int y, int zoom)
{
    double n = M_PI - 2.0 * M_PI * y / static_cast<double>(1 << zoom);
    return 180.0 / M_PI * atan(0.5 * (exp(n) - exp(-n)));
}

void MapTileLoader::getMapTile(int zoom, int x, int y, QRect rectImg)
{
    auto fileDir = QString("/%1/%2").arg(zoom).arg(x);
    auto fileName = QString("/%1.png").arg(y);
    auto fileFullPath = MAP_LOCAL_ROOT + fileDir + fileName;
    auto rectCoord = QRectF(convPixToCoord(zoom, x, y, rectImg, rectImg.topLeft()),
                            convPixToCoord(zoom, x, y, rectImg, rectImg.bottomRight()));

    if (!QDir().exists(MAP_LOCAL_ROOT + fileDir)) {
        if (!QDir().mkpath(MAP_LOCAL_ROOT + fileDir)) {
            qDebug() << "Failed to create directory: " << fileDir;
        }
    }

    if (QFile::exists(fileFullPath)) {
        // load image from exsisting file
        emit downloadedMapTile(fileFullPath, rectImg, rectCoord, zoom);
        curDownloadedCount++;
    }
    else {
        // Download online map tile
        queDownloadReq.push_back(ImageDownloadReq(fileFullPath, rectImg, rectCoord, zoom, urlTileMap + fileDir + fileName));
    }
}

void MapTileLoader::timeoutDownloadQueue()
{
    if (!queDownloadReq.isEmpty()) {
        auto timerNo = sender()->property("NO").toInt();
        auto req = queDownloadReq.takeFirst();
        auto reply = nam[timerNo].get(req.getNetworkReq());
        reply->setProperty("PATH", req.getFilePath());
        reply->setProperty("RECT", req.getImgRect());
        reply->setProperty("ZOOM", req.getZoom());
        reply->setProperty("RECTF", req.getCoordRect());
        connect(reply, &QNetworkReply::finished, this, &MapTileLoader::replyDownloadReq);
    }
}

void MapTileLoader::replyDownloadReq()
{
    auto reply = reinterpret_cast<QNetworkReply *>(sender());

    if (reply->error() == QNetworkReply::NoError) {
        auto filePath = reply->property("PATH").toString();
        auto rectImg = reply->property("RECT").toRect();
        auto rectCoord = reply->property("RECTF").toRectF();
        auto zoom = reply->property("ZOOM").toInt();

        QFile file(filePath);
        if (file.open(QFile::WriteOnly)) {
            file.write(reply->readAll());
            file.close();

            // Complete download image and send file path
            emit downloadedMapTile(filePath, rectImg, rectCoord, zoom);
            curDownloadedCount++;
        }
    }
    else {
        qDebug() << "Reply Error: " << reply->errorString();
    }
    reply->deleteLater();
}
