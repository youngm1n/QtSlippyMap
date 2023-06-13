#ifndef VIEWERMAP_H
#define VIEWERMAP_H

#include <QOpenGLWidget>
#include <QWidget>

#include "maptileloader.h"

#define MAX_ZOOM_COUNT  20

class ViewerMap : public QOpenGLWidget
{
    Q_OBJECT
public:
    ViewerMap(QWidget *parent = nullptr);

    void setUrlTileMap(const QString &newUrlTileMap);
    void setCurrentLocation(float newCurrentLat, float newCurrentLon);

private:
    void updateMapTiles();
    QPointF convPixToCoord(QPoint pt);

private slots:
    void downloadedMapTile(QString imgFilePath, QRect rectScr, int zoom);

signals:
    void updateCurrentLocation(float latitude, float longitude);

private:
    // Map tile parameters
    MapTileLoader mapTileLoader;
    QMap<QString, QRect> mapTiles[MAX_ZOOM_COUNT];

    // Position (real lat/lon)
    QPointF centerLatLon;
    QRectF rectCurrentLatLon;
    QPointF mouseLatLon;


    // drawing parameters
    int currentZoom;
    int previousZoom;
    bool dragMap;
    QPoint dragMapStart;

    // QObject interface
public:
    bool eventFilter(QObject *watched, QEvent *event);

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
