/*
 *  Copyright (c) 2009 Cyrille Berger <cberger@cberger.net>
 *  Copyright (c) 2014 Sven Langkamp <sven.langkamp@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
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

#include "kis_canvas2.h"
#include <kis_view2.h>
#include "kis_image.h"
#include "kis_paint_device.h"
#include "kis_signal_compressor.h"

OverviewWidget::OverviewWidget(QWidget * parent)
    : QWidget(parent)
    , m_compressor(new KisSignalCompressor(500, KisSignalCompressor::POSTPONE, this))
    , m_canvas(0)
    , m_dragging(false)
{
    setMouseTracking(true);
    connect(m_compressor, SIGNAL(timeout()), SLOT(startUpdateCanvasProjection()));
}

OverviewWidget::~OverviewWidget()
{
}

void OverviewWidget::setCanvas(KoCanvasBase * canvas)
{
    if (m_canvas) {
        m_canvas->image()->disconnect(this);
    }

    m_canvas = dynamic_cast<KisCanvas2*>(canvas);
    KIS_ASSERT_RECOVER_RETURN(m_canvas);

    connect(m_canvas->image(), SIGNAL(sigImageUpdated(QRect)), m_compressor, SLOT(start()), Qt::UniqueConnection);
    
    connect(m_canvas->canvasController()->proxyObject, SIGNAL(canvasOffsetXChanged(int)),
            this, SLOT(update()));
    m_compressor->start();
}

QSize OverviewWidget::calculatePreviewSize()
{
    QSize imageSize(m_canvas->image()->bounds().size());
    imageSize.scale(size(), Qt::KeepAspectRatio);
    return imageSize;
}

QPointF OverviewWidget::previewOrigin()
{
    return QPointF((width() - m_pixmap.width())/2.0f, (height() - m_pixmap.height())/2.0f);
}

QPolygonF OverviewWidget::previewPolygon()
{
    if (m_canvas) {
        const KisCoordinatesConverter* converter = m_canvas->coordinatesConverter();
        QPolygonF canvasPoly = QPolygonF(QRectF(m_canvas->canvasWidget()->rect()));
        QPolygonF imagePoly = converter->widgetToImage<QPolygonF>(canvasPoly);
        
        QTransform imageToPreview = imageToPreviewTransform();
      
        return imageToPreview.map(imagePoly);
    }
    return QPolygonF();
}

QTransform OverviewWidget::imageToPreviewTransform()
{
    QTransform imageToPreview;
    imageToPreview.scale(calculatePreviewSize().width()/(float)m_canvas->image()->width(),
                            calculatePreviewSize().height()/(float)m_canvas->image()->height());
    return imageToPreview;
}

void OverviewWidget::startUpdateCanvasProjection()
{
    if (!m_canvas) return;

    KisImageSP image = m_canvas->image();
    QSize previewSize = calculatePreviewSize();

    if (isVisible() && previewSize.isValid()) {
        QImage img =
            image->projection()->
            createThumbnail(previewSize.width(), previewSize.height(), image->bounds());

        m_pixmap = QPixmap::fromImage(img);
    }
    update();
}

void OverviewWidget::showEvent(QShowEvent *event)
{
    Q_UNUSED(event);
    m_compressor->start();
}

void OverviewWidget::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);
    if (m_canvas) {
        if (!m_pixmap.isNull()) {
            QSize newSize = calculatePreviewSize();
            m_pixmap = m_pixmap.scaled(newSize);
        }
        m_compressor->start();
    }
}

void OverviewWidget::mousePressEvent(QMouseEvent* event)
{
    if (m_canvas) {
        QPointF previewPos = event->pos() - previewOrigin();
        
        if (previewPolygon().containsPoint(previewPos, Qt::WindingFill)) {
            m_lastPos = previewPos;
            m_dragging = true;
        }
    }
    event->accept();
    update();
}

void OverviewWidget::mouseMoveEvent(QMouseEvent* event)
{
    if (m_dragging) {
        QPointF previewPos = event->pos() - previewOrigin();

        // position is mapped from preview image->image->canvas coordinates
        QTransform previewToImage = imageToPreviewTransform().inverted();
        const KisCoordinatesConverter* converter = m_canvas->coordinatesConverter();

        QPointF lastImagePos = previewToImage.map(m_lastPos);
        QPointF newImagePos = previewToImage.map(previewPos);
        
        QPointF lastWidgetPos = converter->imageToWidget<QPointF>(lastImagePos);
        QPointF newWidgetPos = converter->imageToWidget<QPointF>(newImagePos);
        
        QPointF diff = newWidgetPos - lastWidgetPos;
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
    float delta = event->delta();
    
    if (delta > 0) {
        m_canvas->view()->zoomController()->zoomAction()->zoomIn();
    } else {
        m_canvas->view()->zoomController()->zoomAction()->zoomOut();
    }
}


void OverviewWidget::paintEvent(QPaintEvent* event)
{
    QWidget::paintEvent(event);
    
    if (m_canvas) {
        QPainter p(this);
        p.translate(previewOrigin());
        
        p.drawPixmap(0, 0, m_pixmap.width(), m_pixmap.height(), m_pixmap);


        QRect r = rect().translated(-previewOrigin().toPoint());
        QPolygonF outline;
        outline << r.topLeft() << r.topRight() << r.bottomRight() << r.bottomLeft();

        QPen pen;
        pen.setColor(QColor(Qt::red));
        pen.setStyle(Qt::DashLine);

        p.setPen(pen);
        p.drawPolygon(outline.intersected(previewPolygon()));

        pen.setStyle(Qt::SolidLine);
        p.setPen(pen);
        p.drawPolygon(previewPolygon());
    }
}

