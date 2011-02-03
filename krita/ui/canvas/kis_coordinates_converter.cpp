/*
 *  Copyright (c) 2010 Dmitry Kazakov <dimula73@gmail.com>
 *  Copyright (c) 2011 Silvio Heinrich <plassy@web.de>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "kis_coordinates_converter.h"

#include <QTransform>
#include <KoViewConverter.h>

#include <kis_config.h>
#include <kis_image.h>


struct KisCoordinatesConverter::Private
{
    Private():
        isXAxisMirrored(false), isYAxisMirrored(false) { }
    
    KisImageWSP image;
    
    bool    isXAxisMirrored;
    bool    isYAxisMirrored;
    QSizeF  canvasWidgetSize;
    QPointF shiftAfterZoom;
    QPointF documentOffset;
    QPoint  documentOrigin;
    
    QTransform imageToDocument;
    QTransform postprocessingTransform;
    QTransform zoom;
    QTransform rotation;
    QTransform widgetToViewport;
};

void KisCoordinatesConverter::recalculateTransformations() const
{
    if(!m_d->image.isValid()) return;
    
    m_d->imageToDocument = QTransform::fromScale(1 / m_d->image->xRes(),
                                                 1 / m_d->image->yRes());
    
    QRectF irect = imageRectInWidgetPixels();
    QRectF wrect = QRectF(QPoint(0,0), m_d->canvasWidgetSize);
    QRectF rrect = irect & wrect;

    QTransform reversedTransform = flakeToWidgetTransform().inverted();
    QRectF     canvasBounds      = reversedTransform.mapRect(rrect);
    QPointF    offset            = canvasBounds.topLeft();

    m_d->widgetToViewport = reversedTransform * QTransform::fromTranslate(-offset.x(), -offset.y());
}


KisCoordinatesConverter::KisCoordinatesConverter()
    : m_d(new Private) { }

KisCoordinatesConverter::~KisCoordinatesConverter()
{
    delete m_d;
}

void KisCoordinatesConverter::setCanvasWidgetSize(QSize size)
{
    m_d->canvasWidgetSize = size;
    recalculateTransformations();
}

void KisCoordinatesConverter::setImage(KisImageWSP image)
{
    m_d->image = image;
    recalculateTransformations();
}

void KisCoordinatesConverter::setDocumentOrigin(const QPoint &origin)
{
    m_d->documentOrigin = origin;
    recalculateTransformations();
}

void KisCoordinatesConverter::setDocumentOffset(const QPoint &offset)
{
    QPointF diff = m_d->documentOffset - offset;
    
    m_d->documentOffset = offset;
    m_d->postprocessingTransform *= QTransform::fromTranslate(diff.x(), diff.y());
    recalculateTransformations();
}

QPoint KisCoordinatesConverter::documentOrigin() const
{
    return m_d->documentOrigin;
}

QPoint KisCoordinatesConverter::documentOffset() const
{
    return QPoint(int(m_d->documentOffset.x()), int(m_d->documentOffset.y()));
}

void KisCoordinatesConverter::setZoom(qreal zoom)
{
    KoZoomHandler::setZoom(zoom);
    
    qreal cx = m_d->canvasWidgetSize.width()  / 2.0;
    qreal cy = m_d->canvasWidgetSize.height() / 2.0;
    
    qreal zoomX, zoomY;
    KoZoomHandler::zoom(&zoomX, &zoomY);
    
    m_d->postprocessingTransform *= QTransform::fromTranslate(-cx,-cy);
    m_d->postprocessingTransform *= m_d->zoom.inverted();
    
    m_d->zoom = QTransform::fromScale(zoomX, zoomY);
    
    m_d->postprocessingTransform *= m_d->zoom;
    m_d->postprocessingTransform *= QTransform::fromTranslate(cx,cy);
    
    recalculateTransformations();
}

void KisCoordinatesConverter::rotate(QPointF center, qreal angle)
{
    QTransform rot;
    rot.rotate(angle);
    
    m_d->rotation                *= rot;
    m_d->postprocessingTransform *= QTransform::fromTranslate(-center.x(),-center.y());
    m_d->postprocessingTransform *= rot;
    m_d->postprocessingTransform *= QTransform::fromTranslate(center.x(), center.y());
    recalculateTransformations();
}

void KisCoordinatesConverter::mirror(QPointF center, bool mirrorXAxis, bool mirrorYAxis, bool keepOrientation)
{
    qreal      scaleX = (m_d->isYAxisMirrored ^ mirrorYAxis) ? -1.0 : 1.0;
    qreal      scaleY = (m_d->isXAxisMirrored ^ mirrorXAxis) ? -1.0 : 1.0;
    QTransform mirror = QTransform::fromScale(scaleX, scaleY);
    
    m_d->postprocessingTransform *= QTransform::fromTranslate(-center.x(),-center.y());
    
    if(keepOrientation)
        m_d->postprocessingTransform *= m_d->rotation.inverted();
    
    m_d->postprocessingTransform *= mirror;
    
    if(keepOrientation)
        m_d->postprocessingTransform *= m_d->rotation;
    
    m_d->postprocessingTransform *= QTransform::fromTranslate(center.x(),center.y());
    m_d->isXAxisMirrored = mirrorXAxis;
    m_d->isYAxisMirrored = mirrorYAxis;
    recalculateTransformations();
}

void KisCoordinatesConverter::resetRotation(QPointF center)
{
    m_d->postprocessingTransform *= QTransform::fromTranslate(-center.x(),-center.y());
    m_d->postprocessingTransform *= m_d->rotation.inverted();
    m_d->postprocessingTransform *= QTransform::fromTranslate(center.x(),center.y());
    m_d->rotation.reset();
    
    recalculateTransformations();
}

QPoint KisCoordinatesConverter::updateOffsetAfterTransform() const {
    QPointF newOffset = -imageRectInWidgetPixels().topLeft();
    QPointF shift     = newOffset - m_d->documentOffset;
    
    m_d->documentOffset = newOffset;
    return shift.toPoint();
}

QTransform KisCoordinatesConverter::imageToWidgetTransform() const{
    return m_d->imageToDocument * m_d->postprocessingTransform;
}

QTransform KisCoordinatesConverter::imageToDocumentTransform() const {
    return m_d->imageToDocument;
}

QTransform KisCoordinatesConverter::flakeToWidgetTransform() const {
    return m_d->zoom.inverted() * m_d->postprocessingTransform;
}

QTransform KisCoordinatesConverter::documentToWidgetTransform() const
{
    return m_d->postprocessingTransform;
}

QTransform KisCoordinatesConverter::viewportToWidgetTransform() const {
    return m_d->widgetToViewport.inverted();
}

QTransform KisCoordinatesConverter::imageToViewportTransform() const {
    return m_d->imageToDocument * m_d->postprocessingTransform * m_d->widgetToViewport;
}

void KisCoordinatesConverter::getQPainterCheckersInfo(QTransform *transform,
                                                      QPointF *brushOrigin,
                                                      QPolygonF *polygon) const
{
    KisConfig cfg;
    if (cfg.scrollCheckers()) {
        *transform = viewportToWidgetTransform();
        *polygon = imageRectInViewportPixels();
        *brushOrigin = imageToViewport(QPointF(0,0));
    }
    else {
        *transform = QTransform();
        *polygon = viewportToWidget(QPolygonF(imageRectInViewportPixels()));
        *brushOrigin = QPoint(0,0);
    }
}

void KisCoordinatesConverter::getOpenGLCheckersInfo(QTransform *textureTransform,
                                                    QTransform *modelTransform,
                                                    QRectF *textureRect,
                                                    QRectF *modelRect) const
{
    KisConfig cfg;
    QRectF viewportRect = imageRectInViewportPixels();

    if(cfg.scrollCheckers()) {
        *textureTransform = QTransform();
        *textureRect = QRectF(0, 0, viewportRect.width(),viewportRect.height());
    }
    else {
        *textureTransform = m_d->widgetToViewport.inverted();
        *textureRect = viewportRect;
    }

    *modelTransform = viewportToWidgetTransform();
    *modelRect = viewportRect;
}


QPointF KisCoordinatesConverter::imageCenterInWidgetPixel() const
{
    if(!m_d->image.isValid())
        return QPointF();
    
    QPolygonF poly = imageToWidget(QPolygon(m_d->image->bounds()));
    return (poly[0] + poly[1] + poly[2] + poly[3]) / 4.0;
}

// these functions return a bounding rect if the canvas is rotated

QRectF KisCoordinatesConverter::imageRectInWidgetPixels() const
{
    if(!m_d->image.isValid()) return QRectF();
    return imageToWidget(m_d->image->bounds());
}

QRectF KisCoordinatesConverter::imageRectInViewportPixels() const
{
    if(!m_d->image.isValid()) return QRectF();
    return imageToViewport(m_d->image->bounds());
}

QSizeF KisCoordinatesConverter::imageSizeInFlakePixels() const
{
    if(!m_d->image.isValid()) return QSizeF();

    qreal scaleX, scaleY;
    imageScale(&scaleX, &scaleY);
    QSize imageSize = m_d->image->size();

    return QSizeF(imageSize.width() * scaleX, imageSize.height() * scaleY);
}

QRectF KisCoordinatesConverter::widgetRectInFlakePixels() const
{
    return widgetToFlake(QRectF(QPoint(0,0), m_d->canvasWidgetSize));
}

QPointF KisCoordinatesConverter::flakeCenterPoint() const
{
    QRectF widgetRect = widgetRectInFlakePixels();
    return QPointF(widgetRect.left() + widgetRect.width() / 2,
                   widgetRect.top() + widgetRect.height() / 2);
}

QPointF KisCoordinatesConverter::widgetCenterPoint() const
{
    return QPointF(m_d->canvasWidgetSize.width() / 2.0, m_d->canvasWidgetSize.height() / 2.0);
}


void KisCoordinatesConverter::imageScale(qreal *scaleX, qreal *scaleY) const
{
    // get the x and y zoom level of the canvas
    qreal zoomX, zoomY;
    KoZoomHandler::zoom(&zoomX, &zoomY);

    // Get the KisImage resolution
    qreal resX = m_d->image->xRes();
    qreal resY = m_d->image->yRes();

    // Compute the scale factors
    *scaleX = zoomX / resX;
    *scaleY = zoomY / resY;
}
