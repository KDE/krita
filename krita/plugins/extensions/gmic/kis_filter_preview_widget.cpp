#include "kis_filter_preview_widget.h"
#include <kis_canvas_widget_base.h>
#include <kis_config.h>
#include <QPaintEvent>
#include <QPainter>
#include <QWheelEvent>

#include <QDebug>

KisFilterPreviewWidget::KisFilterPreviewWidget(QWidget* parent): QWidget(parent)
{
    KisConfig cfg;
    // for preview make it smaller than on canvas
    qint32 checkSize = qMax(1,cfg.checkSize() / 2);
    QImage checkImage = KisCanvasWidgetBase::createCheckersImage(checkSize);
    m_checkBrush = QBrush(checkImage);
    m_pixmap = QPixmap::fromImage(checkImage);
}

KisFilterPreviewWidget::~KisFilterPreviewWidget()
{

}

void KisFilterPreviewWidget::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);
    QPainter gc(this);
    gc.fillRect(m_pixmap.rect(), m_checkBrush);
    gc.drawPixmap(0,0,m_pixmap);


    QPen pen(Qt::black, 1, Qt::SolidLine);
    gc.setPen(pen);
    gc.drawRect(m_pixmap.rect().adjusted(0,0,-1,-1));

}

QSize KisFilterPreviewWidget::sizeHint() const
{
    return QSize(1, 1);
}

void KisFilterPreviewWidget::setImage(const QImage& img)
{
    m_pixmap = QPixmap::fromImage(img);
    update();
}

