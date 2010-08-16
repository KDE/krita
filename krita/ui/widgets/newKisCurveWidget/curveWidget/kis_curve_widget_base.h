#ifndef KIS_CURVE_WIDGET_BASE_H
#define KIS_CURVE_WIDGET_BASE_H

#include <QtGui/QWidget>

class QPainter;

class KisCurveWidgetBase : public QWidget
{
    Q_OBJECT

public:
    KisCurveWidgetBase(QWidget *parent = 0);
    ~KisCurveWidgetBase();

protected:
    void paintEvent(QPaintEvent *);
    void mousePressEvent(QMouseEvent *);
    void mouseMoveEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);
    void mouseDoubleClickEvent(QMouseEvent *);

    void paintBlips(QPainter* painter);

    QList<QPointF> m_points;
    QMatrix m_converterMatrix;

private:
    int m_currentPoint;
};

#endif // KIS_CURVE_WIDGET_BASE_H
