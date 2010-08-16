#include "kis_linear_curve_widget.h"

#include <QPainter>
#include <QPainterPath>

KisLinearCurveWidget::KisLinearCurveWidget(QWidget *parent) :
    KisCurveWidgetBase(parent)
{
}

void KisLinearCurveWidget::paintEvent(QPaintEvent *e) {
    KisCurveWidgetBase::paintEvent(e);

    QPainter painter(this);
    painter.setMatrix(m_converterMatrix);
    painter.setRenderHint(QPainter::Antialiasing);

    QPainterPath path;
    path.moveTo(m_points.first());

    for(int i=1; i<m_points.size(); i++) {
        path.lineTo(m_points.at(i));
    }

    painter.drawPath(path);

    paintBlips(&painter);
}
