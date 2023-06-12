#include "maptileloader.h"

#include <QDir>
#include <QFile>

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

    connect(&timerNamReq, &QTimer::timeout, this, &MapTileLoader::timeoutNamReq);
    timerNamReq.start(10);
}

void MapTileLoader::setUrlTileMap(const QString &newUrlTileMap)
{
    // Set tile map server base
    urlTileMap = newUrlTileMap;
}

void MapTileLoader::getMapFromCoordinate(double lat, double lon, int zoom, QRect rect)
{
    auto tileCenter = QPoint(longitudeToTileX(lon, zoom), latitudeToTileY(lat, zoom));
    auto tileCount = QPoint(ceil(rect.width() / 2.0 / static_cast<double>(MAP_TILE_PIX_SIZE)),
                              ceil(rect.height() / 2.0 / static_cast<double>(MAP_TILE_PIX_SIZE)));
    auto tileStart = tileCenter - tileCount;
    auto tileEnd = tileCenter + tileCount;

    auto scrStart = QPoint(rect.center() - QPoint(tileCount.x() * MAP_TILE_PIX_SIZE, tileCount.y() * MAP_TILE_PIX_SIZE));

    auto scrY = scrStart.y();
    for (int y = tileStart.y(); y < tileEnd.y(); y++) {
        auto scrX = scrStart.x();
        for (int x = tileStart.x(); x < tileEnd.x(); x++) {
            getMapTile(zoom, x, y, QRect(scrX, scrY, MAP_TILE_PIX_SIZE, MAP_TILE_PIX_SIZE));
            scrX += MAP_TILE_PIX_SIZE;
        }
        scrY += MAP_TILE_PIX_SIZE;
    }
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

bool MapTileLoader::getMapTile(int zoom, int x, int y, QRect rectScr)
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
        emit downloadedMapTile(fileFullPath, rectScr);
    }
    else {
        // Download online map tile
        QString url = urlTileMap + fileDir + fileName;

        auto *nam = new QNetworkAccessManager(this);
        nam->setProperty("PATH", fileFullPath);
        nam->setProperty("URL", url);
        nam->setProperty("SCREEN_RECT", rectScr);
        connect(nam, &QNetworkAccessManager::finished, this, &MapTileLoader::finishedTileDownload);
        mutexNamReq.lock();
        nams.push_back(nam);
        mutexNamReq.unlock();
    }

    return ret;
}

void MapTileLoader::timeoutNamReq()
{
    mutexNamReq.lock();

    if (!nams.isEmpty()) {
        auto nam = nams.takeFirst();

        QNetworkRequest req;
        req.setUrl(QUrl(nam->property("URL").toString()));
        req.setRawHeader("User-Agent", "The Qt Company (Qt) Graphics Dojo 1.0");
        nam->get(req);
    }

    mutexNamReq.unlock();
}

void MapTileLoader::finishedTileDownload(QNetworkReply *reply)
{
    auto nam = reinterpret_cast<QNetworkAccessManager *>(sender());
    if (reply->error() == QNetworkReply::NoError) {
        auto fileFullPath = nam->property("PATH").toString();

        QFile file(fileFullPath);
        if (file.open(QFile::WriteOnly)) {
            file.write(reply->readAll());
            file.close();

            // Complete download image and send file path
            emit downloadedMapTile(fileFullPath, nam->property("SCREEN_RECT").toRect());
        }
    }
    else {
        qDebug() << reply->errorString();
    }
    reply->deleteLater();
    disconnect(nam, &QNetworkAccessManager::finished, this, &MapTileLoader::finishedTileDownload);
}
