#include "kis_curve_widget.h"

#include <QPainter>
#include <QPainterPath>
#include <QVector2D>

KisCurveWidget::KisCurveWidget(QWidget *parent)
    : QWidget(parent)
{
    resize(500, 500);
}

KisCurveWidget::~KisCurveWidget()
{

}


void KisCurveWidget::paintEvent(QPaintEvent *)
{
    QList<QPoint> points;
    points.append(QPoint(0,0));
//    points.append(QPoint(100,10));
//    points.append(QPoint(150,100));
//    points.append(QPoint(180,70));
//    points.append(QPoint(210,280));
//    points.append(QPoint(255,0));
    points.append(QPoint(40, 260));
    points.append(QPoint(300,300));

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.translate(100, 100);
    QPainterPath path;
    path.moveTo(points.first());
//    path.lineTo(100, 200);
//    path.lineTo(300, 300);
//    path.quadTo(100,200, 300,300);

    QList<QPair<QVector2D, QVector2D> > controlPoints;
//    controlPoints.append(QPair<QVector2D, QVector2D>(QVector2D(0,0), QVector2D(1,1)));

    for(int i=0; i<points.size(); i++) {
        QVector2D last(points.at((i-1)>=0?(i-1):0));
        QVector2D current(points.at(i));
        QVector2D next(points.at((i+1)<points.size()?(i+1):i));

        QVector2D tangent = next-last;
        tangent.normalize();

        QVector2D ctrlPt1(current - (current.x()-last.x())*0.5*tangent);
        QVector2D ctrlPt2(current + (next.x()-current.x())*0.5*tangent);

        controlPoints.append(QPair<QVector2D, QVector2D>(ctrlPt1, ctrlPt2));
    }

//    controlPoints.append(QPair<QVector2D, QVector2D>(QVector2D(299,299), QVector2D(301,301)));

//    path.lineTo(points.at());
    for(int i=1; i<points.size(); i++) {
        path.cubicTo(controlPoints.at(i-1).second.toPoint(), controlPoints.at(i).first.toPoint(), points.at(i));
    }
//    path.lineTo(points.last());


    painter.drawPath(path);


    for(int i=0; i<points.size(); i++) {
        painter.drawEllipse(points.at(i), 5, 5);
    }

    for(int i=0; i<controlPoints.size(); i++) {
        painter.drawEllipse(controlPoints.at(i).first.toPoint(), 2, 2);
        painter.drawEllipse(controlPoints.at(i).second.toPoint(), 3, 3);
        painter.drawEllipse(controlPoints.at(i).second.toPoint(), 1, 1);
    }
}
