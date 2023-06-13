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

    void setType(int newType);

    float getValue() const;
    void setValue(float newValue);

private slots:
    void changeDmsValues();

private:
    Ui::FormCoordinate *ui;

    int type;
    float value;
};

#endif // FORMCOORDINATE_H
