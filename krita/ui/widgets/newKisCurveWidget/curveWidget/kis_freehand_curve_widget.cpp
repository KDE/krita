#include "kis_freehand_curve_widget.h"

#include <QPainter>
#include <QMouseEvent>
#include <QVector2D>

#include <QDebug>

KisFreehandCurveWidget::KisFreehandCurveWidget(QWidget *parent) :
    KisCurveWidgetBase(parent), m_lastPointX(-1)
{
    reset();
}

QList<QPointF> KisFreehandCurveWidget::controlPoints() const
{
    QList<QPointF> retPoints;

    for (QMap<int, int>::const_iterator iter=m_points.begin(); iter!=(m_points.end()); iter++) {
        retPoints.append(QPointF(iter.key(), iter.value()));
    }

    return retPoints;
}

void KisFreehandCurveWidget::setControlPoints(const QList<QPointF> &points)
{
    deletePoints(0, CURVE_RANGE);

    for(int i=0; i<points.size(); i++) {
        m_points.insert(points.at(i).x(), points.at(i).y());
    }
}

void KisFreehandCurveWidget::reset()
{
    m_points.clear();

    m_points.insert(0,0);
    m_points.insert(CURVE_RANGE, CURVE_RANGE);

    update();
}

void KisFreehandCurveWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    paintBackground(&painter);

    painter.setMatrix(m_converterMatrix);

    QPainterPath path;
    QMap<int, int>::iterator firstPoint = m_points.begin();
    path.moveTo(firstPoint.key(), firstPoint.value());


    for (QMap<int, int>::iterator iter=m_points.begin(); iter!=(m_points.end()); iter++) {
        path.lineTo(iter.key(), iter.value());
    }

    painter.drawPath(path);
}

void KisFreehandCurveWidget::deletePoints(int fromX, int toX)
{
    int start = qMin(fromX, toX);
    int end = qMax(fromX, toX);

    QList<int> keys = m_points.keys();
    for(int i=0; i<keys.size(); i++) {
        if(keys.at(i)>start && keys.at(i)<end)
            m_points.remove(keys.at(i));
    }
}

void KisFreehandCurveWidget::mousePressEvent(QMouseEvent *event)
{
    QVector2D mousePos(m_converterMatrix.inverted().map(event->posF()));
    mousePos.setX(qBound(0., mousePos.x(), CURVE_RANGE));
    mousePos.setY(qBound(0., mousePos.y(), CURVE_RANGE));

    if(event->button()==Qt::LeftButton) {
        m_lastPointX = mousePos.x();
    }
}

void KisFreehandCurveWidget::mouseMoveEvent(QMouseEvent *event)
{
    QVector2D mousePos(m_converterMatrix.inverted().map(event->posF()));
    mousePos.setX(qBound(0., mousePos.x(), CURVE_RANGE));
    mousePos.setY(qBound(0., mousePos.y(), CURVE_RANGE));

    if(event->buttons()&Qt::LeftButton) {
        deletePoints(m_lastPointX, mousePos.x());
        m_points.insert(mousePos.x(), mousePos.y());
        m_lastPointX=mousePos.x();
        update();
    }
}
