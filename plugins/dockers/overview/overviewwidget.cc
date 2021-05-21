/*
 *  Copyright (c) 2009 Cyrille Berger <cberger@cberger.net>
 *  Copyright (c) 2014 Sven Langkamp <sven.langkamp@gmail.com>
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */


#include "overviewwidget.h"

#include <QMouseEvent>
#include <QPainter>
#include <QCursor>

#include <KoCanvasController.h>
#include <KoZoomController.h>

#include <kis_canvas2.h>
#include <KisViewManager.h>
#include <kis_image.h>
#include <kis_signal_compressor.h>
#include <kis_config.h>
#include "kis_idle_watcher.h"
#include <QApplication>
#include "OverviewThumbnailStrokeStrategy.h"
#include <kis_display_color_converter.h>

OverviewWidget::OverviewWidget(QWidget * parent)
    : QWidget(parent)
    , m_canvas(0)
    , m_dragging(false)
    , m_imageIdleWatcher(250)
{
    setMouseTracking(true);
    KisConfig cfg(true);
    slotThemeChanged();
    recalculatePreviewDimensions();
}

OverviewWidget::~OverviewWidget()
{
}

void OverviewWidget::setCanvas(KoCanvasBase * canvas)
{
    if (m_canvas) {
        m_canvas->image()->disconnect(this);
        m_canvas->displayColorConverter()->disconnect(this);
    }

    m_canvas = dynamic_cast<KisCanvas2*>(canvas);

    if (m_canvas) {
        m_imageIdleWatcher.setTrackedImage(m_canvas->image());

        connect(&m_imageIdleWatcher, &KisIdleWatcher::startedIdleMode, this, &OverviewWidget::generateThumbnail);

        connect(m_canvas->image(), SIGNAL(sigImageUpdated(QRect)),SLOT(startUpdateCanvasProjection()));
        connect(m_canvas->image(), SIGNAL(sigSizeChanged(QPointF,QPointF)),SLOT(startUpdateCanvasProjection()));
        connect(m_canvas->displayColorConverter(), SIGNAL(displayConfigurationChanged()), SLOT(startUpdateCanvasProjection()));

        connect(m_canvas->canvasController()->proxyObject, SIGNAL(canvasOffsetXChanged(int)), this, SLOT(update()), Qt::UniqueConnection);
        connect(m_canvas->viewManager()->mainWindow(), SIGNAL(themeChanged()), this, SLOT(slotThemeChanged()));
        generateThumbnail();
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
    m_imageIdleWatcher.startCountdown();
}

void OverviewWidget::showEvent(QShowEvent *event)
{
    Q_UNUSED(event);
    m_imageIdleWatcher.startCountdown();
}

void OverviewWidget::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);
    if (m_canvas) {
        if (!m_oldPixmap.isNull()) {
            recalculatePreviewDimensions();
            m_pixmap = m_oldPixmap.scaled(m_previewSize, Qt::KeepAspectRatio, Qt::FastTransformation);
        }
        m_imageIdleWatcher.startCountdown();
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
    m_dragging = false;
    event->accept();
    update();
}

void OverviewWidget::wheelEvent(QWheelEvent* event)
{
    if (m_canvas) {
        float delta = event->delta();

        if (delta > 0) {
            m_canvas->viewManager()->zoomController()->zoomAction()->zoomIn();
        } else {
            m_canvas->viewManager()->zoomController()->zoomAction()->zoomOut();
        }
    }
}

void OverviewWidget::generateThumbnail()
{
    if (isVisible()) {
        QMutexLocker locker(&mutex);
        if (m_canvas) {
            recalculatePreviewDimensions();
            if(m_previewSize.isValid()){
                KisImageSP image = m_canvas->image();

                /**
                 * Compress the updates: if our previous stroke is still running
                 * then idle watcher has missed something. Just poke it and wait
                 * for the next event
                 */
                if (!strokeId.isNull()) {
                    m_imageIdleWatcher.startCountdown();
                    return;
                }

                const KoColorProfile *profile =
                   m_canvas->displayColorConverter()->monitorProfile();
                KoColorConversionTransformation::ConversionFlags conversionFlags =
                    m_canvas->displayColorConverter()->conversionFlags();
                KoColorConversionTransformation::Intent renderingIntent =
                    m_canvas->displayColorConverter()->renderingIntent();

                OverviewThumbnailStrokeStrategy* stroke;
                stroke = new OverviewThumbnailStrokeStrategy(image->projection(), image->bounds(), m_previewSize, isPixelArt(), profile, renderingIntent, conversionFlags);

                connect(stroke, SIGNAL(thumbnailUpdated(QImage)), this, SLOT(updateThumbnail(QImage)));

                strokeId = image->startStroke(stroke);
                image->endStroke(strokeId);
            }
        }
    }
}

void OverviewWidget::updateThumbnail(QImage pixmap)
{
    m_pixmap = QPixmap::fromImage(pixmap);
    m_oldPixmap = m_pixmap.copy();
    m_image = pixmap;
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


