#ifndef MAPTILELOADER_H
#define MAPTILELOADER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QUrl>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QEventLoop>
#include <QImage>

#include "global.h"

class MapTileLoader : public QObject
{
    Q_OBJECT
public:
    explicit MapTileLoader(QObject *parent = nullptr);

    void setUrlTileMap(const QString &newUrlTileMap);

    void getMapFromCoordinate(double lonLeft, double latTop, double lonRight, double latBottom, QRectF rect);

private:
    int longitudeToTileX(double lon, int zoom);
    int latitudeToTileY(double lat, int zoom);
    double tileXtoLongitude(int x, int zoom);
    double tileYtoLatitude(int y, int zoom);

    bool getMapTile(int zoom, int x, int y);

private slots:
    void finishedTileDownload(QNetworkReply *reply);

signals:
    void downloadedMapTile(QString imgFilePath, QRect rectScr);

private:
    QString urlTileMap;
    QMap<int, double> mapLonSizePerZoom;

signals:

};

#endif // MAPTILELOADER_H
