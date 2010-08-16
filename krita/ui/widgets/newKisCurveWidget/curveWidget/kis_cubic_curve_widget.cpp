#include "kis_cubic_curve_widget.h"

#include <QVector2D>
#include <QPainter>
#include <QPainterPath>

KisCubicCurveWidget::KisCubicCurveWidget(QWidget *parent) :
    KisCurveWidgetBase(parent)
{
}

void KisCubicCurveWidget::paintEvent(QPaintEvent *e)
{
    KisCurveWidgetBase::paintEvent(e);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setMatrix(m_converterMatrix);
    QPainterPath path;
    path.moveTo(m_points.first());

    QList<QPair<QVector2D, QVector2D> > controlPoints;

    for(int i=1; i<m_points.size()-1; i++) {
        QVector2D last(m_points.at((i-1)>=0?(i-1):0));
        QVector2D current(m_points.at(i));
        QVector2D next(m_points.at((i+1)<m_points.size()?(i+1):i));

        QVector2D tangent = next-last;
        tangent.normalize();

        QVector2D ctrlPt1(current - (current.x()-last.x())*0.5*tangent);
        QVector2D ctrlPt2(current + (next.x()-current.x())*0.5*tangent);

        controlPoints.append(QPair<QVector2D, QVector2D>(ctrlPt1, ctrlPt2));
    }

    if(controlPoints.size()>0) {
        controlPoints.prepend(controlPoints.first());
        controlPoints.first().second=controlPoints.first().first;

        controlPoints.append(controlPoints.last());
        controlPoints.last().first=controlPoints.last().second;
    }
    else {
        controlPoints.append(QPair<QVector2D, QVector2D>(QVector2D(m_points.first()), QVector2D(m_points.first())));
        controlPoints.append(QPair<QVector2D, QVector2D>(QVector2D(m_points.last()), QVector2D(m_points.last())));
    }


    for(int i=1; i<m_points.size(); i++) {
        path.cubicTo(controlPoints.at(i-1).second.toPointF(), controlPoints.at(i).first.toPointF(), m_points.at(i));
    }

    painter.drawPath(path);

    paintBlips(&painter);

    // debug output:
//    for(int i=0; i<controlPoints.size(); i++) {
//        painter.drawEllipse(controlPoints.at(i).first.toPoint(), 2, 2);
//        painter.drawText(controlPoints.at(i).first.toPoint(), QString::number(i));
//        painter.drawEllipse(controlPoints.at(i).second.toPoint(), 3, 3);
//        painter.drawEllipse(controlPoints.at(i).second.toPoint(), 1, 1);
//        painter.drawText(controlPoints.at(i).second.toPoint(), QString::number(i));
//    }
}
