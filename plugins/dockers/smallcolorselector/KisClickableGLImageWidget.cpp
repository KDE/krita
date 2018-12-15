#include "KisClickableGLImageWidget.h"

#include <QMouseEvent>
#include <QPainter>
#include "kis_algebra_2d.h"
#include <kis_debug.h>

KisClickableGLImageWidget::KisClickableGLImageWidget(QWidget *parent)
    : KisGLImageWidget(parent)
{
}

KisClickableGLImageWidget::KisClickableGLImageWidget(QSurfaceFormat::ColorSpace colorSpace, QWidget *parent)
    : KisGLImageWidget(colorSpace, parent)
{
}

KisClickableGLImageWidget::~KisClickableGLImageWidget()
{
}

void KisClickableGLImageWidget::setHandlePaintingStrategy(HandlePaintingStrategy *strategy)
{
    m_handleStrategy.reset(strategy);
}

void KisClickableGLImageWidget::setUseHandleOpacity(bool value)
{
    m_useHandleOpacity = value;
    update();
}

QPointF KisClickableGLImageWidget::normalizedPos() const
{
    return m_normalizedClickPoint;
}

void KisClickableGLImageWidget::setNormalizedPos(const QPointF &pos, bool update)
{
    m_normalizedClickPoint = KisAlgebra2D::clampPoint(pos, QRectF(0,0,1.0,1.0));
    if (update) {
        this->update();
    }
}

void KisClickableGLImageWidget::paintEvent(QPaintEvent *event)
{
    KisGLImageWidget::paintEvent(event);

    if (m_handleStrategy) {
        QPainter p(this);
        m_handleStrategy->drawHandle(&p, m_normalizedClickPoint, rect(), m_useHandleOpacity);
    }
}

void KisClickableGLImageWidget::mousePressEvent(QMouseEvent *event)
{
    KisGLImageWidget::mousePressEvent(event);

    if (!event->isAccepted()) {
        event->accept();
        m_normalizedClickPoint = normalizePoint(event->localPos());
        emit selected(m_normalizedClickPoint);

        if (m_handleStrategy) {
            update();
        }
    }
}

void KisClickableGLImageWidget::mouseReleaseEvent(QMouseEvent *event)
{
    KisGLImageWidget::mouseReleaseEvent(event);

    if (!event->isAccepted()) {
        event->accept();
        m_normalizedClickPoint = normalizePoint(event->localPos());
        emit selected(m_normalizedClickPoint);

        if (m_handleStrategy) {
            update();
        }
    }
}

void KisClickableGLImageWidget::mouseMoveEvent(QMouseEvent *event)
{
    KisGLImageWidget::mouseMoveEvent(event);

    if (!event->isAccepted()) {
        event->accept();
        m_normalizedClickPoint = normalizePoint(event->localPos());
        emit selected(m_normalizedClickPoint);

        if (m_handleStrategy) {
            update();
        }
    }
}

QPointF KisClickableGLImageWidget::normalizePoint(const QPointF &pos) const
{
    const QPointF croppedPoint = KisAlgebra2D::clampPoint(pos, rect());
    return QPointF(croppedPoint.x() / width(), croppedPoint.y() / height());
}


namespace {
QPen outerHandlePen(bool useOpacity) {
    // opacity works inexpectedly in HDR mode, so let the user switch it off
    return QPen(QColor(0, 0, 0, useOpacity ? 180 : 255), 0);
}
QPen innerHandlePen(bool useOpacity) {
    // opacity works inexpectedly in HDR mode, so let the user switch it off
    return QPen(QColor(255, 255, 255, useOpacity ? 180 : 255), 0);
}
}

void KisClickableGLImageWidget::VerticalLineHandleStrategy::drawHandle(QPainter *p, const QPointF &normalizedPoint, const QRect &rect, bool useOpacity)
{
    const QPointF pos = KisAlgebra2D::relativeToAbsolute(normalizedPoint, rect);
    const int x = std::floor(pos.x());

    p->setPen(outerHandlePen(useOpacity));
    p->drawLine(x, rect.top(), x, rect.bottom());
    p->setPen(innerHandlePen(useOpacity));
    p->drawLine(x + 1, rect.top(), x + 1, rect.bottom());
}

void KisClickableGLImageWidget::CircularHandleStrategy::drawHandle(QPainter *p, const QPointF &normalizedPoint, const QRect &rect, bool useOpacity)
{
    const QPointF pos = KisAlgebra2D::relativeToAbsolute(normalizedPoint, rect);

    p->setRenderHint(QPainter::Antialiasing);
    p->setPen(outerHandlePen(useOpacity));
    p->drawEllipse(pos, 5, 5);

    p->setPen(innerHandlePen(useOpacity));
    p->drawEllipse(pos, 4, 4);
}
