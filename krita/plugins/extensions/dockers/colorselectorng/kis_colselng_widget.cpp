#include "kis_colselng_widget.h"
#include "kis_colselng_my_paint_shade_selector.h"
#include <QPainter>

KisColSelNgWidget::KisColSelNgWidget(QWidget *parent) :
    QWidget(parent)
{
    setMinimumHeight(50);
}

void KisColSelNgWidget::paintEvent(QPaintEvent *) {
    QPainter painter(this);
    KisColSelNgMyPaintShadeSelector myPaintShadeSel;

    painter.drawImage(0,0, myPaintShadeSel.getSelector(QColor(200,30,30)));
}
