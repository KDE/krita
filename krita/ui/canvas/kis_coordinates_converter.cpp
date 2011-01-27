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

#include <QPolygonF>
#include <QTransform>
#include <QSizeF>

#include <KoViewConverter.h>

#include <kis_config.h>
#include <kis_image.h>


struct KisCoordinatesConverter::Private
{
    Private():
        mirrorXAxis(false), mirrorYAxis(false) { }
    
    KisImageWSP image;
    
    bool   mirrorXAxis;
    bool   mirrorYAxis;
    QSize  canvasWidgetSize;
    QPoint documentOffset;
    QPoint documentOrigin;
    
    QTransform imageToDocument;
    QTransform postprocessingTransform;
    QTransform transform;
    QTransform widgetToViewport;
};

void KisCoordinatesConverter::recalculateTransformations() const
{
    if(!m_d->image.isValid()) return;
    
    m_d->imageToDocument = QTransform::fromScale(1 / m_d->image->xRes(),
                                                 1 / m_d->image->yRes());
    
    qreal scaleX = m_d->mirrorYAxis ? -1.0 : 1.0;
    qreal scaleY = m_d->mirrorXAxis ? -1.0 : 1.0;
    qreal cx     = qreal(m_d->canvasWidgetSize.width()) / 2.0;
    qreal cy     = qreal(m_d->canvasWidgetSize.height()) / 2.0;
    
    m_d->transform.reset();
    m_d->transform *= QTransform::fromTranslate(-cx,-cy) * QTransform::fromScale(scaleX, scaleY) * QTransform::fromTranslate(cx,cy);
    m_d->transform *= m_d->postprocessingTransform;

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
#include <iostream>
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
    return m_d->documentOffset;
}

void KisCoordinatesConverter::setZoom(qreal zoom)
{
    qreal zFactor = zoom / KoZoomHandler::zoom();
    qreal cx      = qreal(m_d->canvasWidgetSize.width()) / 2.0;
    qreal cy      = qreal(m_d->canvasWidgetSize.height()) / 2.0;
    
    KoZoomHandler::setZoom(zoom);
    
    m_d->postprocessingTransform *=
        QTransform::fromTranslate(-cx,-cy) * QTransform::fromScale(zFactor,zFactor) * QTransform::fromTranslate(cx,cy);
    
//     m_d->documentOffset.setX(-m_d->postprocessingTransform.m31());
//     m_d->documentOffset.setY(-m_d->postprocessingTransform.m32());
    recalculateTransformations();
}

void KisCoordinatesConverter::rotate(qreal angle)
{
    QTransform rotation;
    rotation.rotate(angle);
    
    qreal cx = qreal(m_d->canvasWidgetSize.width())  / 2.0;
    qreal cy = qreal(m_d->canvasWidgetSize.height()) / 2.0;
    
    m_d->postprocessingTransform *=
        QTransform::fromTranslate(-cx,-cy) * rotation * QTransform::fromTranslate(cx,cy);
    
    recalculateTransformations();
}

void KisCoordinatesConverter::mirror(bool mirrorXAxis, bool mirrorYAxis)
{
    m_d->mirrorXAxis = mirrorXAxis;
    m_d->mirrorYAxis = mirrorYAxis;
    recalculateTransformations();
}

void KisCoordinatesConverter::resetRotation()
{
    qreal  zoom = KoZoomHandler::zoom();
    
    m_d->postprocessingTransform.reset();
    m_d->postprocessingTransform *= QTransform::fromScale(zoom, zoom);
    m_d->postprocessingTransform *= QTransform::fromTranslate(-m_d->documentOffset.x(), -m_d->documentOffset.y());
    
    recalculateTransformations();
}

QTransform KisCoordinatesConverter::imageToWidgetTransform() const{
    return m_d->imageToDocument * m_d->transform;
}

QTransform KisCoordinatesConverter::imageToDocumentTransform() const {
    return m_d->imageToDocument;
}

QTransform KisCoordinatesConverter::flakeToWidgetTransform() const {
    return m_d->transform;
}

QTransform KisCoordinatesConverter::documentToWidgetTransform() const
{
    return m_d->transform;
}

QTransform KisCoordinatesConverter::widgetToViewportTransform() const {
    return m_d->widgetToViewport;
}

QTransform KisCoordinatesConverter::imageToViewportTransform() const {
    return m_d->imageToDocument * m_d->transform * m_d->widgetToViewport;
}

void KisCoordinatesConverter::getQPainterCheckersInfo(QTransform *transform,
                                                      QPointF *brushOrigin,
                                                      QPolygonF *polygon) const
{
    KisConfig cfg;
    if (cfg.scrollCheckers()) {
        *transform = widgetToViewportTransform().inverted();
        *polygon = imageRectInViewportPixels();
        *brushOrigin = imageToViewport(QPointF(0,0));
    }
    else {
        *transform = QTransform();
        *polygon = widgetToViewportTransform().inverted().map(imageRectInViewportPixels());
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

    *modelTransform = widgetToViewportTransform().inverted();
    *modelRect = viewportRect;
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

void KisCoordinatesConverter::imageScale(qreal *scaleX, qreal *scaleY) const
{
    // get the x and y zoom level of the canvas
    qreal zoomX, zoomY;
    zoom(&zoomX, &zoomY);

    // Get the KisImage resolution
    qreal resX = m_d->image->xRes();
    qreal resY = m_d->image->yRes();

    // Compute the scale factors
    *scaleX = zoomX / resX;
    *scaleY = zoomY / resY;
}
