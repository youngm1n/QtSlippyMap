#ifndef VIEWERMAP_H
#define VIEWERMAP_H

#include <QOpenGLWidget>
#include <QWidget>

class ViewerMap : public QOpenGLWidget
{
    Q_OBJECT
public:
    ViewerMap(QWidget *parent = nullptr);
};

#endif // VIEWERMAP_H
