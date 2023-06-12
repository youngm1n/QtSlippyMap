#include "maptileloader.h"

#include <QDir>
#include <QFile>

#define MAP_LOCAL_ROOT      "./Map"
#define MAP_TILE_PIX_SIZE   256

MapTileLoader::MapTileLoader(QObject *parent)
    : QObject{parent}
{
    QDir().mkdir(MAP_LOCAL_ROOT);
    for (int zoom = 0; zoom < 20; zoom++) {
        mapLonSizePerZoom.insert(zoom, 360.0 / pow(2, zoom));
    }
}

void MapTileLoader::setUrlTileMap(const QString &newUrlTileMap)
{
    // Set tile map server base
    urlTileMap = newUrlTileMap;
}

void MapTileLoader::getMapFromCoordinate(double lonLeft, double latTop, double lonRight, double latBottom, QRectF rect)
{
    auto xTileCount = ceil(rect.width() / static_cast<double>(MAP_TILE_PIX_SIZE));
    auto lonGap = (lonRight - lonLeft) / xTileCount;

    // Find Proper Zoom Level
    int zoom = 0;
    for (QMap<int, double>::iterator iter = mapLonSizePerZoom.begin(); iter != mapLonSizePerZoom.end(); ++iter) {
        if (lonGap > iter.value()) {
            zoom = iter.key();
            break;
        }
    }

    if (zoom > 0) {
        // Generated temp map (without cropping)
        QPoint tileNoStart(longitudeToTileX(lonLeft, zoom), latitudeToTileY(latTop, zoom));
        QPoint tileNoEnd(longitudeToTileX(lonRight, zoom) + 1, latitudeToTileY(latBottom, zoom) + 1);

        for (int y = tileNoStart.y(); y <= tileNoEnd.y(); y++) {
            for (int x = tileNoStart.x(); x <= tileNoEnd.x(); x++) {
                getMapTile(zoom, x, y);
            }
        }
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

bool MapTileLoader::getMapTile(int zoom, int x, int y)
{
    bool ret = false;
    QString fileDir = QString("/%1/%2").arg(zoom).arg(x);
    QString fileName = QString("/%1.png").arg(y);
    QString fileFullPath = MAP_LOCAL_ROOT + fileDir + fileName;

    QRect rectScr;

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
        nam->setProperty("SCREEN_RECT", rectScr);
        connect(nam, &QNetworkAccessManager::finished, this, &MapTileLoader::finishedTileDownload);

        QNetworkRequest req;
        req.setUrl(QUrl(url));
        req.setRawHeader("User-Agent", "The Qt Company (Qt) Graphics Dojo 1.0");
        nam->get(req);
    }

    return ret;
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
//    disconnect(nam, &QNetworkAccessManager::finished, this, &MapTileLoader::finishedTileDownload);
}
