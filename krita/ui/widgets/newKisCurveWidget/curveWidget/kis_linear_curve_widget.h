#ifndef KIS_LINEAR_CURVE_WIDGET_H
#define KIS_LINEAR_CURVE_WIDGET_H

#include "kis_curve_widget_base.h"

class KisLinearCurveWidget : public KisCurveWidgetBase
{
    Q_OBJECT
public:
    explicit KisLinearCurveWidget(QWidget *parent = 0);

protected:
    void paintEvent(QPaintEvent *);

};

#endif // KIS_LINEAR_CURVE_WIDGET_H
