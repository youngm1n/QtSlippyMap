#ifndef FORMCOORDINATE_H
#define FORMCOORDINATE_H

#include <QWidget>

namespace Ui {
class FormCoordinate;
}

class FormCoordinate : public QWidget
{
    Q_OBJECT

public:
    explicit FormCoordinate(QWidget *parent = nullptr);
    ~FormCoordinate();

    void setShowType(int newType);
    void setLatLonType(int newLatLonType);

    float getValue() const;
    void setValue(float newValue);

private:
    void connectItems(bool con);

private slots:
    void changeDmsValues();

private:
    Ui::FormCoordinate *ui;

    int showType;
    int latLonType;
    float value;
};

#endif // FORMCOORDINATE_H
