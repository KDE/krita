#ifndef COLORSELECTORNGWIDGET_H
#define COLORSELECTORNGWIDGET_H

#include <QWidget>

class KisColSelNgWidget : public QWidget
{
Q_OBJECT
public:
    explicit KisColSelNgWidget(QWidget *parent = 0);

signals:

public slots:
protected:
    void paintEvent(QPaintEvent *);

};

#endif // COLORSELECTORNGWIDGET_H
