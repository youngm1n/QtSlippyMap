#ifndef MAPTILELOADER_H
#define MAPTILELOADER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QUrl>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QEventLoop>
#include <QImage>
#include <QTimer>
#include <QQueue>
#include <QMutex>

#include "global.h"

// https://wiki.openstreetmap.org/wiki/Slippy_map_tilenames

class ImageDownloadReq
{
public:
    explicit ImageDownloadReq(QString newFilePath, QRect newRect, QString url) {
        filePath = newFilePath;
        rect = newRect;
        req.setUrl(QUrl(url));
        req.setRawHeader("User-Agent", "The Qt Company (Qt) Graphics Dojo 1.0");
    }

    QNetworkRequest getNetworkReq() { return req; }
    QString getFilePath()   { return filePath; }
    QRect getImgRect()      { return rect; }

private:
    QNetworkRequest req;
    QString filePath;
    QRect rect;
};

class MapTileLoader : public QObject
{
    Q_OBJECT
public:
    explicit MapTileLoader(QObject *parent = nullptr);

    void setUrlTileMap(const QString &newUrlTileMap);
    int startDownloadTiles(double lat, double lon, int zoom, QRect rect);

private:
    int longitudeToTileX(double lon, int zoom);
    int latitudeToTileY(double lat, int zoom);
    double tileXtoLongitude(int x, int zoom);
    double tileYtoLatitude(int y, int zoom);

    bool getMapTile(int zoom, int x, int y, QRect rectScr);

private slots:
    void timeoutDownloadQueue();

signals:
    void downloadedMapTile(QString imgFilePath, QRect rectScr);

private:
    QString urlTileMap;
    QMap<int, double> mapLonSizePerZoom;

    QNetworkAccessManager nam;
    QQueue<ImageDownloadReq> queDownloadReq;
    QTimer timerDownloadQue;

signals:

};

#endif // MAPTILELOADER_H
