#ifndef VIEWERMAP_H
#define VIEWERMAP_H

#include <QOpenGLWidget>
#include <QWidget>
#include <QMenu>

#include "maptileloader.h"
#include "dialogmeasuredistance.h"

typedef QMap<QString, QPair<QRect, QRectF>> MAP_TILES;

class ViewerMap : public QOpenGLWidget
{
    Q_OBJECT
public:
    ViewerMap(QWidget *parent = nullptr);

    void setInitTileMap(const QString &url, const int maxZoomLevel);
    void setCurrentLocation(float newCurrentLat, float newCurrentLon);

public slots:
    void setShowGridCenter(bool showGrid);
    void setShowGridTiles(bool showGrid);

private:
    void drawMapTiles(QPainter &p);
    void drawGridForMapTiles(QPainter &p);
    void drawGridForCenter(QPainter &p);
    void drawMeasurement(QPainter &p);
    void drawInfomation(QPainter &p);
    void updateMapTiles();

    bool convPixToCoord(const QPoint &pix, QPointF &coord);
    QPointF convCoordToPix(const QPointF &coord);

    void addMeasurePoint(QPoint pos);

private slots:
    void downloadedMapTile(QString imgFilePath, QRect rectScr, QRectF rectCoord, int zoom);
    void triggeredMenuAction(bool checked = false);

signals:
    void updateCurrentLocation(float latitude, float longitude);

private:
    // Map tile parameters
    MapTileLoader mapTileLoader;
    MAP_TILES *mapTiles;
    QImage imgTempMap;
    QImage imgCacheMap;
    QRect rectCacheMap;

    // Position (real lat/lon)
    QPointF centerLatLon;
    QSizeF sizeScrLatLon;
    QPointF mouseLatLon;

    // drawing parameters
    int currentZoom;
    int previousZoom;
    int maxZoom;
    bool dragMap;
    QPoint dragMapStart;
    bool showGridCenter;
    bool showGridTiles;
    int currentMouseBtn;

    // Right click menu
    QMenu menu;

    // Measurement
    DialogMeasureDistance *dlgMeas;

    // QObject interface
public:
    bool eventFilter(QObject *watched, QEvent *event);

protected:
//    void paintGL();
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
