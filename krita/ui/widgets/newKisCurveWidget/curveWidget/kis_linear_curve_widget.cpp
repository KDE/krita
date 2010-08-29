#include "kis_linear_curve_widget.h"

#include <QPainter>
#include <QPainterPath>

KisLinearCurveWidget::KisLinearCurveWidget(QWidget *parent) :
    KisCurveWidgetBase(parent)
{
}

void KisLinearCurveWidget::paintEvent(QPaintEvent *e) {

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    paintBackground(&painter);

    painter.setMatrix(m_converterMatrix);

    QPainterPath path;
    path.moveTo(m_points.first());

    for(int i=1; i<m_points.size(); i++) {
        path.lineTo(m_points.at(i));
    }

    painter.drawPath(path);

    paintBlips(&painter);
}
