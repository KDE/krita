#include "kis_curve_widget_base.h"

#include <QPainter>
#include <QPainterPath>
#include <QVector2D>
#include <QMouseEvent>

KisCurveWidgetBase::KisCurveWidgetBase(QWidget *parent)
    : QWidget(parent), m_currentPoint(-1)
{
    resize(500, 500);

    m_points.append(QPointF(0, 0));
    m_points.append(QPoint(300, 300));

    m_converterMatrix.translate(100, 400);
    m_converterMatrix.scale(1, -1);
}

KisCurveWidgetBase::~KisCurveWidgetBase()
{

}


void KisCurveWidgetBase::paintEvent(QPaintEvent *)
{
}


void KisCurveWidgetBase::mousePressEvent(QMouseEvent *event)
{
    if(event->button()==Qt::LeftButton) {
        QVector2D mousePos(m_converterMatrix.inverted().map(event->posF()));
        for(int i=0; i<m_points.size(); i++) {
            QVector2D pointPos(m_points.at(i));
            if((mousePos-pointPos).lengthSquared()<36) {
                m_currentPoint=i;
                break;
            }
        }
    }
}

void KisCurveWidgetBase::mouseMoveEvent(QMouseEvent *event)
{
    if(m_currentPoint!=-1) {
        QPointF mousePos = m_converterMatrix.inverted().map(event->posF());
        if(m_currentPoint!=0 && m_currentPoint!=m_points.size()-1) {
            if(!(rect().adjusted(-20, -20, 20, 20).contains(event->pos()))) {
                m_points.removeAt(m_currentPoint);
                m_currentPoint = -1;
                update();
                return;
            }
            else if(mousePos.x()<m_points.at(m_currentPoint-1).x()) {
                m_points[m_currentPoint].setX(m_points.at(m_currentPoint-1).x());
            }
            else if (mousePos.x()>m_points.at(m_currentPoint+1).x()) {
                m_points[m_currentPoint].setX(m_points.at(m_currentPoint+1).x());
            }
            else {
                m_points[m_currentPoint].setX(mousePos.x());
            }
        }
        m_points[m_currentPoint].setY(mousePos.y());
        update();
    }
}

void KisCurveWidgetBase::mouseReleaseEvent(QMouseEvent *event)
{
    if(event->button()==Qt::LeftButton)
        m_currentPoint=-1;
}

bool pointCompare (const QPointF &p1, const QPointF &p2)
{
    if(p1.x()<p2.x()) return true;
    if(p1.x()==p2.x() && p1.y()<p2.y()) return true;
    return false;
}

void KisCurveWidgetBase::mouseDoubleClickEvent(QMouseEvent *event)
{
    QVector2D mousePos(m_converterMatrix.inverted().map(event->posF()));
    for(int i=0; i<m_points.size(); i++) {
        if((QVector2D(m_points.at(i))-mousePos).lengthSquared()<36) {
            if(i>0 && i<m_points.size()-1) { // don't add a point, if double clicking on start or end
                m_points.removeAt(i);
                update();
            }
            return;
        }
    }
    m_points.append(mousePos.toPointF());
    qSort(m_points.begin(), m_points.end(), pointCompare);
    update();
}

void KisCurveWidgetBase::paintBlips(QPainter *painter)
{
    for(int i=0; i<m_points.size(); i++) {
        painter->drawEllipse(m_points.at(i), 5, 5);
    }
}
