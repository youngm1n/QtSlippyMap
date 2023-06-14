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
#include <QThread>

// https://wiki.openstreetmap.org/wiki/Slippy_map_tilenames

class ImageDownloadReq
{
public:
    explicit ImageDownloadReq(QString newFilePath, QRect newRectImg, QRectF newRectCoord, int newZoom, QString url) {
        filePath = newFilePath;
        rectImg = newRectImg;
        rectCoord = newRectCoord;
        zoom = newZoom;
        req.setUrl(QUrl(url));
        req.setRawHeader("User-Agent", "The Qt Company (Qt) Graphics Dojo 1.0");
    }

    QNetworkRequest getNetworkReq() { return req; }
    QString getFilePath()   { return filePath; }
    QRect getImgRect()      { return rectImg; }
    QRectF getCoordRect()   { return rectCoord; }
    int getZoom()           { return zoom; }

private:
    QNetworkRequest req;
    QString filePath;
    QRect rectImg;
    QRectF rectCoord;
    int zoom;
};

#define MAX_NAM_COUNT   2

class MapTileLoader : public QObject
{
    Q_OBJECT
public:
    explicit MapTileLoader(QObject *parent = nullptr);

    void setUrlTileMap(const QString &newUrlTileMap);
    void resetDownloadQueue();
    QSizeF startDownloadTiles(QPointF center, int zoom, QRect rect);
    bool isDownloading();

private:
    int longitudeToTileX(double lon, int zoom);
    int latitudeToTileY(double lat, int zoom);
    double tileXtoLongitude(int x, int zoom);
    double tileYtoLatitude(int y, int zoom);

    void getMapTile(int zoom, int x, int y, QRect rectScr);
    QPointF convPixToCoord(const int &zoom, const int &x, const int &y, QRect scrRect, QPoint pix);
    int rescaleTileIndex(const int &no, const int &maxNo);

private slots:
    void timeoutDownloadQueue();
    void replyDownloadReq();

signals:
    void downloadedMapTile(QString imgFilePath, QRect rectImg, QRectF rectCoord, int zoom);

private:
    QString urlTileMap;
    QMap<int, double> mapLonSizePerZoom;

    QNetworkAccessManager nam[MAX_NAM_COUNT];   // Crashed, when the number is over 30 (windows is ok, but only linux)--> Why?
    QTimer timerDownloadQue[MAX_NAM_COUNT];
    QQueue<ImageDownloadReq> queDownloadReq;

    int totalDownloadCount;
    int curDownloadedCount;

signals:
};

#endif // MAPTILELOADER_H
