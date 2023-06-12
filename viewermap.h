#ifndef VIEWERMAP_H
#define VIEWERMAP_H

#include <QOpenGLWidget>
#include <QWidget>

#include "maptileloader.h"

#include "global.h"

class ViewerMap : public QOpenGLWidget
{
    Q_OBJECT
public:
    ViewerMap(QWidget *parent = nullptr);

    void setUrlTileMap(const QString &newUrlTileMap);

private:
    void updateMapTiles();
    void convScreenPosToLatLon(QPointF pos, double &lat, double &lon);
    void convScreenPosToLatLon(QPointF pos, double &lat, double &lon, double &dist, double &bearing);
    void getDestinationLatLon(double srcLat, double srcLon, double distance, double bearing, double &dstLat, double &dstLon);

private slots:
    void downloadedMapTile(QString imgFilePath, QRect rectScr);

private:
    MapTileLoader mapTileLoader;
    double currentLat;
    double currentLon;

    double realDistPerScrPix;
    int realRadius;     // Unit: meter

    QRectF rectMapDraw; // x2 size for rotating

    QMap<QString, QRect> mapTiles;

    // QObject interface
public:
    bool eventFilter(QObject *watched, QEvent *event);

    // QWidget interface
    void setCurrentLocation(double newCurrentLat, double newCurrentLon);

protected:
    void paintEvent(QPaintEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseDoubleClickEvent(QMouseEvent *event);
    void wheelEvent(QWheelEvent *event);

    // QOpenGLWidget interface
protected:
    void resizeGL(int w, int h);
};

#endif // VIEWERMAP_H
