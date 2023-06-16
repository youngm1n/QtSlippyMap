#ifndef DIALOGMEASUREDISTANCE_H
#define DIALOGMEASUREDISTANCE_H

#include <QDialog>

namespace Ui {
class DialogMeasureDistance;
}

class DialogMeasureDistance : public QDialog
{
    Q_OBJECT

public:
    explicit DialogMeasureDistance(QWidget *parent = nullptr);
    ~DialogMeasureDistance();

    void addMeasurePoint(QPointF pt);
    QList<QPointF> getPointList() const;
    QList<float> getDistanceList() const;
    bool isEmpty();

private:
    void setTotalDistance(float newTotalDist);
    float getDistanceBetweenCoordinates(QPointF coordSrc, QPointF coordDst);

signals:
    void clearList();

private:
    Ui::DialogMeasureDistance *ui;
    QList<QPointF> points;
    QList<float> dists;
};

#endif // DIALOGMEASUREDISTANCE_H
