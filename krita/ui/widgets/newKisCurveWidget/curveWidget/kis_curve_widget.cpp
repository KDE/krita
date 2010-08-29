#include "kis_curve_widget.h"

#include <QHBoxLayout>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>

#include "kis_cubic_curve_widget.h"
#include "kis_linear_curve_widget.h"
#include "kis_freehand_curve_widget.h"



KisCurveWidget::KisCurveWidget(QWidget *parent)
    : QWidget(parent)
{
    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->addWidget(new KisFreehandCurveWidget(this));
    layout->addWidget(new KisCubicCurveWidget(this));
    layout->addWidget(new KisLinearCurveWidget(this));

    resize(1500, 500);
}

KisCurveWidget::~KisCurveWidget()
{

}

