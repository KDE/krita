#ifndef KIS_CURVE_WIDGET_H
#define KIS_CURVE_WIDGET_H

#include <QtGui/QWidget>

class KisCurveWidget : public QWidget
{
    Q_OBJECT

public:
    KisCurveWidget(QWidget *parent = 0);
    ~KisCurveWidget();

protected:
    void paintEvent(QPaintEvent *);
};

#endif // KIS_CURVE_WIDGET_H
