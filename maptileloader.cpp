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
}

void MapTileLoader::setUrlTileMap(const QString &newUrlTileMap)
{
    // Set tile map server base
    urlTileMap = newUrlTileMap;
}

QRectF MapTileLoader::startDownloadTiles(double centerLat, double centerLon, int zoom, QRect rect)
{
    int maxTileNo = static_cast<int>(pow(2, zoom)) - 1;
    auto tileNoCenter = QPoint(longitudeToTileX(centerLon, zoom), latitudeToTileY(centerLat, zoom));
    auto tileNoCount = QPoint(ceil(rect.width() / 2.0 / static_cast<double>(MAP_TILE_PIX_SIZE)) + 1,
                              ceil(rect.height() / 2.0 / static_cast<double>(MAP_TILE_PIX_SIZE)) + 1);
    auto tileNoStart = tileNoCenter - tileNoCount;
    auto tileNoEnd = tileNoCenter + tileNoCount;

    // Rescale start and end no by max tile number
    if (tileNoStart.x() < 0) {
        tileNoStart.setX(0);
    }
    if (tileNoStart.x() > maxTileNo) {
        tileNoStart.setX(maxTileNo);
    }
    if (tileNoStart.y() < 0) {
        tileNoStart.setY(0);
    }
    if (tileNoStart.y() > maxTileNo) {
        tileNoStart.setY(maxTileNo);
    }

    if (tileNoEnd.x() < 0) {
        tileNoEnd.setX(0);
    }
    if (tileNoEnd.x() > maxTileNo) {
        tileNoEnd.setX(maxTileNo);
    }
    if (tileNoEnd.y() < 0) {
        tileNoEnd.setY(0);
    }
    if (tileNoEnd.y() > maxTileNo) {
        tileNoEnd.setY(maxTileNo);
    }

    auto tileRealCenter = QRectF(QPointF(tileXtoLongitude(tileNoCenter.x(), zoom), tileYtoLatitude(tileNoCenter.y(), zoom)),
                                 QPointF(tileXtoLongitude(tileNoCenter.x() + 1, zoom), tileYtoLatitude(tileNoCenter.y() + 1, zoom)));

    auto scrDelta = QPoint((centerLon - tileRealCenter.x()) / tileRealCenter.width() * MAP_TILE_PIX_SIZE,
                           (centerLat - tileRealCenter.y()) / tileRealCenter.height() * MAP_TILE_PIX_SIZE);
    auto scrStart = QPoint(rect.center() - QPoint(tileNoCount.x() * MAP_TILE_PIX_SIZE, tileNoCount.y() * MAP_TILE_PIX_SIZE)) - scrDelta;

    queDownloadReq.clear();
    auto scrY = scrStart.y();
    for (int y = tileNoStart.y(); y < tileNoEnd.y(); y++) {
        auto scrX = scrStart.x();
        for (int x = tileNoStart.x(); x < tileNoEnd.x(); x++) {
            getMapTile(zoom, x, y, QRect(scrX, scrY, MAP_TILE_PIX_SIZE, MAP_TILE_PIX_SIZE));
            scrX += MAP_TILE_PIX_SIZE;
        }
        scrY += MAP_TILE_PIX_SIZE;
    }

    // return the screen's real coordinate
    QRectF rectScrCoord;
    rectScrCoord.setSize(QSizeF(rect.width() * tileRealCenter.width() / MAP_TILE_PIX_SIZE,
                            -rect.height() * abs(tileRealCenter.height()) / MAP_TILE_PIX_SIZE));
    rectScrCoord.moveCenter(QPointF(centerLon, centerLat));
    return rectScrCoord;
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

bool MapTileLoader::getMapTile(int zoom, int x, int y, QRect rectImg)
{
    auto ret = false;
    auto fileDir = QString("/%1/%2").arg(zoom).arg(x);
    auto fileName = QString("/%1.png").arg(y);
    auto fileFullPath = MAP_LOCAL_ROOT + fileDir + fileName;

    if (!QDir().exists(MAP_LOCAL_ROOT + fileDir)) {
        if (!QDir().mkpath(MAP_LOCAL_ROOT + fileDir)) {
            qDebug() << "Failed to create directory: " << fileDir;
        }
    }

    if (QFile::exists(fileFullPath)) {
        // load image from exsisting file
        emit downloadedMapTile(fileFullPath, rectImg, zoom);
        ret = true;
    }
    else {
        // Download online map tile
        queDownloadReq.push_back(ImageDownloadReq(fileFullPath, rectImg, zoom, urlTileMap + fileDir + fileName));
    }

    return ret;
}

void MapTileLoader::timeoutDownloadQueue()
{
    if (!queDownloadReq.isEmpty()) {
        auto timerNo = sender()->property("NO").toInt();
        auto req = queDownloadReq.takeFirst();
        auto reply = nam[timerNo].get(req.getNetworkReq());

        QEventLoop loop;
        connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
        loop.exec();

        if (reply->error() == QNetworkReply::NoError) {
            QFile file(req.getFilePath());
            if (file.open(QFile::WriteOnly)) {
                file.write(reply->readAll());
                file.close();

                // Complete download image and send file path
                emit downloadedMapTile(req.getFilePath(), req.getImgRect(), req.getZoom());
            }
        }
    }
}
