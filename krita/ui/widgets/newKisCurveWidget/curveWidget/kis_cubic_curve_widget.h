#ifndef KIS_CUBIC_CURVE_WIDGET_H
#define KIS_CUBIC_CURVE_WIDGET_H

#include "kis_curve_widget_base.h"

class KisCubicCurveWidget : public KisCurveWidgetBase
{
    Q_OBJECT
public:
    explicit KisCubicCurveWidget(QWidget *parent = 0);

protected:
    void paintEvent(QPaintEvent *);

};

#endif // KIS_CUBIC_CURVE_WIDGET_H
