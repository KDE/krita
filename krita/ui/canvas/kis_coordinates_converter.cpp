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


struct KisCoordinatesConverter::Private {
    KisImageWSP image;
    QSize canvasWidgetSize;
    QPoint documentOffset;
    QPoint documentOrigin;
    QTransform postprocessingTransform;

    QTransform imageToDocument;
    QTransform documentToFlake;
    QTransform flakeToPostprocessedFlake;
    QTransform postprocessedFlakeToWidget;
    QTransform widgetToViewport;
};

void KisCoordinatesConverter::recalculateTransformations() const
{
    if(!m_d->image.isValid()) return;

    m_d->imageToDocument = QTransform::fromScale(1 / m_d->image->xRes(),
                                                 1 / m_d->image->yRes());

    qreal zoomX, zoomY;
    zoom(&zoomX, &zoomY);
    m_d->documentToFlake = QTransform::fromScale(zoomX, zoomY);

    // Make new coordinate system not go to negative values
    QSizeF  flakeSize = imageSizeInFlakePixels();
    QPointF shift     = -m_d->postprocessingTransform.mapRect(QRectF(QPointF(0,0), flakeSize)).topLeft();

    m_d->flakeToPostprocessedFlake =
        m_d->postprocessingTransform * QTransform::fromTranslate(shift.x(), shift.y());

    m_d->postprocessedFlakeToWidget =
        QTransform::fromTranslate(-m_d->documentOffset.x() + m_d->documentOrigin.x(),
                                  -m_d->documentOffset.y() + m_d->documentOrigin.y());

    QRectF irect = imageRectInWidgetPixels();
    QRectF wrect = QRectF(QPoint(0,0), m_d->canvasWidgetSize);
    QRectF rrect = irect & wrect;

    QTransform reversedTransform = flakeToWidgetTransform().inverted();
    QRectF     canvasBounds      = reversedTransform.mapRect(rrect);
    QPointF    offset            = canvasBounds.topLeft();

    m_d->widgetToViewport = reversedTransform * QTransform::fromTranslate(-offset.x(), -offset.y());

    // qDebug() << "***********";
    // qDebug() << ppVar(m_d->flakeToPostprocessedFlake);
    // qDebug() << ppVar(m_d->postprocessedFlakeToWidget);
    // qDebug() << ppVar(m_d->widgetToViewport);
    // qDebug() << ppVar(documentOrigin());
    // qDebug() << ppVar(irect) << ppVar(wrect) << ppVar(rrect);
    // qDebug() << ppVar(canvasBounds);
    // qDebug() << ppVar(m_d->widgetToViewport.mapRect(irect));
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
    m_d->documentOffset = offset;
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
    KoZoomHandler::setZoom(zoom);
    recalculateTransformations();
}

void KisCoordinatesConverter::rotate(qreal angle)
{
    m_d->postprocessingTransform.rotate(angle);
    recalculateTransformations();
}

void KisCoordinatesConverter::mirror(bool mirrorXAxis, bool mirrorYAxis)
{
    qreal scaleX = mirrorYAxis ? -1.0 : 1.0;
    qreal scaleY = mirrorXAxis ? -1.0 : 1.0;
    m_d->postprocessingTransform.scale(scaleX, scaleY);
    recalculateTransformations();
}

void KisCoordinatesConverter::resetTransformations()
{
    m_d->postprocessingTransform.reset();
    recalculateTransformations();
}

QTransform KisCoordinatesConverter::imageToWidgetTransform() const{
    return m_d->imageToDocument * m_d->documentToFlake * m_d->flakeToPostprocessedFlake * m_d->postprocessedFlakeToWidget;
}

QTransform KisCoordinatesConverter::imageToDocumentTransform() const {
    return m_d->imageToDocument;
}

QTransform KisCoordinatesConverter::flakeToWidgetTransform() const {
    return m_d->flakeToPostprocessedFlake * m_d->postprocessedFlakeToWidget;
}

QTransform KisCoordinatesConverter::documentToWidgetTransform() const
{
    return m_d->documentToFlake * m_d->flakeToPostprocessedFlake * m_d->postprocessedFlakeToWidget;
}

QTransform KisCoordinatesConverter::widgetToViewportTransform() const {
    return m_d->widgetToViewport;
}

QTransform KisCoordinatesConverter::imageToViewportTransform() const {
    return m_d->imageToDocument * m_d->documentToFlake * m_d->flakeToPostprocessedFlake * m_d->postprocessedFlakeToWidget * m_d->widgetToViewport;
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
    return viewportToWidget(imageToViewport(m_d->image->bounds()));
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

QPoint KisCoordinatesConverter::shiftFromFlakeCenterPoint(const QPointF &pt) const
{
    QPointF newWidgetCenter = flakeToWidgetTransform().map(pt);
    QPointF oldWidgetCenter = QPointF(m_d->canvasWidgetSize.width() / 2, m_d->canvasWidgetSize.height() / 2);
    return (newWidgetCenter - oldWidgetCenter).toPoint();
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
