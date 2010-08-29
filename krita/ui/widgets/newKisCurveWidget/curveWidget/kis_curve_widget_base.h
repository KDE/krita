#ifndef KIS_CURVE_WIDGET_BASE_H
#define KIS_CURVE_WIDGET_BASE_H

#include <QtGui/QWidget>

class QPainter;
class QVector2D;


class KisCurveWidgetBase : public QWidget
{
    Q_OBJECT

protected:
    const qreal CURVE_RANGE;

public:
    KisCurveWidgetBase(QWidget *parent = 0);
    ~KisCurveWidgetBase();

    virtual QList<QPointF> controlPoints() const;
    virtual void setControlPoints(const QList<QPointF> &points);

public slots:
    virtual void reset();

protected:
    void mousePressEvent(QMouseEvent *);
    void mouseMoveEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);
    void mouseDoubleClickEvent(QMouseEvent *);
    void resizeEvent(QResizeEvent *);

    void paintBlips(QPainter* painter);
    void paintBackground(QPainter* painter);

    void addPoint(const QVector2D& pos);
    // returns true, if pos is a point, also if the point is not removed.
    bool removePoint(const QVector2D& pos);

    QList<QPointF> m_points;
    QMatrix m_converterMatrix;

private:
    int m_currentPoint;
};

#endif // KIS_CURVE_WIDGET_BASE_H
