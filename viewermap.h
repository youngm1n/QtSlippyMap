#ifndef VIEWERMAP_H
#define VIEWERMAP_H

#include <QOpenGLWidget>
#include <QWidget>

#include "maptileloader.h"

#include "global.h"

#define MAX_ZOOM_COUNT  20

class ViewerMap : public QOpenGLWidget
{
    Q_OBJECT
public:
    ViewerMap(QWidget *parent = nullptr);

    void setUrlTileMap(const QString &newUrlTileMap);

private:
    void updateMapTiles();

private slots:
    void downloadedMapTile(QString imgFilePath, QRect rectScr);

private:
    MapTileLoader mapTileLoader;
    double currentLat;
    double currentLon;

    QRect rectMapDraw; // x2 size for rotating
    int zoom;

    QMap<QString, QRect> mapTiles[MAX_ZOOM_COUNT];

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
