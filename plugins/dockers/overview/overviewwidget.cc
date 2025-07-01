/*
 *  SPDX-FileCopyrightText: 2009 Cyrille Berger <cberger@cberger.net>
 *  SPDX-FileCopyrightText: 2014 Sven Langkamp <sven.langkamp@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */


#include "overviewwidget.h"

#include <QMouseEvent>
#include <QPainter>
#include <QCursor>

#include <KoCanvasController.h>

#include <kis_canvas2.h>
#include <KisViewManager.h>
#include <kis_image.h>
#include <kis_signal_compressor.h>
#include <kis_config.h>
#include <QApplication>
#include "KisImageThumbnailStrokeStrategy.h"
#include <kis_display_color_converter.h>
#include <KisMainWindow.h>
#include "KisIdleTasksManager.h"
#include <KisDisplayConfig.h>


OverviewWidget::OverviewWidget(QWidget * parent)
    : KisWidgetWithIdleTask<QWidget>(parent)
    , m_dragging(false)
{
    setMouseTracking(true);
    KisConfig cfg(true);
    slotThemeChanged();
    recalculatePreviewDimensions();
}

OverviewWidget::~OverviewWidget()
{
}

void OverviewWidget::setCanvas(KisCanvas2 *canvas)
{
    if (m_canvas) {
        m_canvas->image()->disconnect(this);
        m_canvas->displayColorConverter()->disconnect(this);
    }

    KisWidgetWithIdleTask<QWidget>::setCanvas(canvas);

    if (m_canvas) {
        connect(m_canvas->displayColorConverter(), SIGNAL(displayConfigurationChanged()), SLOT(startUpdateCanvasProjection()));
        // TODO: we need a proper "any signal changed" here
        connect(m_canvas->canvasController()->proxyObject, SIGNAL(canvasOffsetChanged()), this, SLOT(update()), Qt::UniqueConnection);
        connect(m_canvas->viewManager()->mainWindow(), SIGNAL(themeChanged()), this, SLOT(slotThemeChanged()), Qt::UniqueConnection);
    }
}

void OverviewWidget::recalculatePreviewDimensions()
{
    if (!m_canvas || !m_canvas->image()) {
        return;
    }

    QSize imageSize(m_canvas->image()->bounds().size());

    const qreal hScale = 1.0 * this->width() / imageSize.width();
    const qreal vScale = 1.0 * this->height() / imageSize.height();

    m_previewScale = qMin(hScale, vScale);
    m_previewSize = imageSize * m_previewScale;
    m_previewOrigin = calculatePreviewOrigin(m_previewSize);

}

KisIdleTasksManager::TaskGuard OverviewWidget::registerIdleTask(KisCanvas2 *canvas)
{
    return
        canvas->viewManager()->idleTasksManager()->
        addIdleTaskWithGuard([this](KisImageSP image) {
            const KisDisplayConfig config = m_canvas->displayColorConverter()->displayConfig();

            // If the widget is presented on a device with a pixel ratio > 1.0, we must compensate for it
            // by increasing the thumbnail's resolution. Otherwise it will appear blurry.
            QSize thumbnailSize = m_previewSize * devicePixelRatioF();

            if ((thumbnailSize.width() > image->width()) || (thumbnailSize.height() > image->height())) {
                thumbnailSize.scale(image->size(), Qt::KeepAspectRatio);
            }

            KisImageThumbnailStrokeStrategy *strategy =
                new KisImageThumbnailStrokeStrategy(image->projection(), image->bounds(), thumbnailSize, isPixelArt(), config.profile, config.intent, config.conversionFlags);

            connect(strategy, SIGNAL(thumbnailUpdated(QImage)), this, SLOT(updateThumbnail(QImage)));

            return strategy;
        });
}

void OverviewWidget::clearCachedState()
{
    m_pixmap = QPixmap();
    m_oldPixmap = QPixmap();
}

bool OverviewWidget::isPixelArt()
{
    return m_previewScale > 1;
}

QPointF OverviewWidget::calculatePreviewOrigin(QSize previewSize)
{
    return QPointF((width() - previewSize.width()) / 2.0f, (height() - previewSize.height()) / 2.0f);
}

QPolygonF OverviewWidget::previewPolygon()
{
    if (m_canvas) {
        const QRectF &canvasRect = QRectF(m_canvas->canvasWidget()->rect());
        return canvasToPreviewTransform().map(canvasRect);
    }
    return QPolygonF();
}

QTransform OverviewWidget::previewToCanvasTransform()
{
    QTransform previewToImage =
            QTransform::fromTranslate(-this->width() / 2.0, -this->height() / 2.0) *
            QTransform::fromScale(1.0 / m_previewScale, 1.0 / m_previewScale) *
            QTransform::fromTranslate(m_canvas->image()->width() / 2.0, m_canvas->image()->height() / 2.0);

    return previewToImage * m_canvas->coordinatesConverter()->imageToWidgetTransform();
}

QTransform OverviewWidget::canvasToPreviewTransform()
{
    return previewToCanvasTransform().inverted();
}

void OverviewWidget::startUpdateCanvasProjection()
{
    triggerCacheUpdate();
}

void OverviewWidget::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);
    if (m_canvas) {
        if (!m_oldPixmap.isNull()) {
            recalculatePreviewDimensions();
            m_pixmap = m_oldPixmap.scaled(m_previewSize, Qt::KeepAspectRatio, Qt::FastTransformation);
        }
        triggerCacheUpdate();
    }
}

void OverviewWidget::mousePressEvent(QMouseEvent* event)
{
    if (m_canvas) {
        QPointF previewPos = event->pos();

        if (!previewPolygon().containsPoint(previewPos, Qt::WindingFill)) {
            const QRect& canvasRect = m_canvas->canvasWidget()->rect();
            const QPointF newCanvasPos = previewToCanvasTransform().map(previewPos) -
                    QPointF(canvasRect.width() / 2.0f, canvasRect.height() / 2.0f);
            m_canvas->canvasController()->pan(newCanvasPos.toPoint());
        }
        m_lastPos = previewPos;
        m_dragging = true;
        Q_EMIT signalDraggingStarted();
    }
    event->accept();
    update();
}

void OverviewWidget::mouseMoveEvent(QMouseEvent* event)
{
    if (m_dragging) {
        QPointF previewPos = event->pos();
        const QPointF lastCanvasPos = previewToCanvasTransform().map(m_lastPos);
        const QPointF newCanvasPos = previewToCanvasTransform().map(event->pos());

        QPointF diff = newCanvasPos - lastCanvasPos;
        m_canvas->canvasController()->pan(diff.toPoint());
        m_lastPos = previewPos;
    }
    event->accept();
}

void OverviewWidget::mouseReleaseEvent(QMouseEvent* event)
{
    if (m_dragging) {
        m_dragging = false;
        Q_EMIT signalDraggingFinished();
    }
    event->accept();
    update();
}

void OverviewWidget::wheelEvent(QWheelEvent* event)
{
    if (m_canvas) {
        if (event->angleDelta().y() > 0) {
            m_canvas->canvasController()->zoomIn();
        } else {
            m_canvas->canvasController()->zoomOut();
        }
    }
}

void OverviewWidget::updateThumbnail(QImage pixmap)
{
    m_pixmap = QPixmap::fromImage(pixmap);
    m_oldPixmap = m_pixmap.copy();
    update();
}

void OverviewWidget::slotThemeChanged()
{
    m_outlineColor = qApp->palette().color(QPalette::Highlight);
}


void OverviewWidget::paintEvent(QPaintEvent* event)
{
    QWidget::paintEvent(event);

    if (m_canvas) {
        recalculatePreviewDimensions();
        QPainter p(this);

        const QRectF previewRect = QRectF(m_previewOrigin, m_previewSize);
        p.drawPixmap(previewRect.toRect(), m_pixmap);

        QRect r = rect();
        QPolygonF outline;
        outline << r.topLeft() << r.topRight() << r.bottomRight() << r.bottomLeft();

        QPen pen;
        pen.setColor(m_outlineColor);
        pen.setStyle(Qt::DashLine);

        p.setPen(pen);
        p.drawPolygon(outline.intersected(previewPolygon()));

        pen.setStyle(Qt::SolidLine);
        p.setPen(pen);
        p.drawPolygon(previewPolygon());

    }
}


