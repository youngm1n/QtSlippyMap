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

class MapTileLoader : public QObject
{
    Q_OBJECT
public:
    explicit MapTileLoader(QObject *parent = nullptr);

    void setUrlTileMap(const QString &newUrlTileMap);

    void getMapFromCoordinate(double lat, double lon, int zoom, QRect rect);

private:
    int longitudeToTileX(double lon, int zoom);
    int latitudeToTileY(double lat, int zoom);
    double tileXtoLongitude(int x, int zoom);
    double tileYtoLatitude(int y, int zoom);

    bool getMapTile(int zoom, int x, int y, QRect rectScr);

private slots:
    void finishedTileDownload(QNetworkReply *reply);
    void timeoutNamReq();

signals:
    void downloadedMapTile(QString imgFilePath, QRect rectScr);

private:
    QString urlTileMap;
    QMap<int, double> mapLonSizePerZoom;

    QQueue<QNetworkAccessManager *> nams;
    QTimer timerNamReq;
    QMutex mutexNamReq;

signals:

};

#endif // MAPTILELOADER_H
