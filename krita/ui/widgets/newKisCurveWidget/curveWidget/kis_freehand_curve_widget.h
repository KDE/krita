#ifndef KIS_FREEHAND_CURVE_WIDGET_H
#define KIS_FREEHAND_CURVE_WIDGET_H

#include "kis_curve_widget_base.h"

#include <QMap>

class KisFreehandCurveWidget : public KisCurveWidgetBase {
public:
    KisFreehandCurveWidget(QWidget *parent = 0);

protected:
    void paintEvent(QPaintEvent *);
    void deletePoints(int fromX, int toX);
    void mousePressEvent(QMouseEvent *e);
    void mouseMoveEvent(QMouseEvent *e);
    void mouseDoubleClickEvent(QMouseEvent *) {} //do nothing, overide superclass
    void mouseReleaseEvent(QMouseEvent *) {}     //same here

private:
    QMap<int, int> m_points;
    int m_lastPointX;
};

#endif // KIS_FREEHAND_CURVE_WIDGET_H
