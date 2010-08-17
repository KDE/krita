/*
 *  Copyright (c) 2010 Dmitry Kazakov <dimula73@gmail.com>
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
    KoViewConverter *viewConverter;
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
    if(!m_d->image) return;

    m_d->imageToDocument = QTransform::fromScale(1 / m_d->image->xRes(),
                                                 1 / m_d->image->yRes());

    qreal zoomX, zoomY;
    m_d->viewConverter->zoom(&zoomX, &zoomY);
    m_d->documentToFlake = QTransform::fromScale(zoomX, zoomY);

    // Make new coordinate system not go to negative values
    QSizeF flakeSize = imageSizeInFlakePixels();
    QPointF shift = -m_d->postprocessingTransform.mapRect(QRectF(QPointF(0,0), flakeSize)).topLeft();

    m_d->flakeToPostprocessedFlake = m_d->postprocessingTransform
        * QTransform::fromTranslate(shift.x(), shift.y());

    m_d->postprocessedFlakeToWidget =
        QTransform::fromTranslate(-m_d->documentOffset.x() + m_d->documentOrigin.x(),
                                  -m_d->documentOffset.y() + m_d->documentOrigin.y());


    QTransform reversedTransform = (m_d->flakeToPostprocessedFlake * m_d->postprocessedFlakeToWidget).inverted();

    QRectF irect = imageRectInWidgetPixels();
    QRectF wrect = QRectF(QPoint(0,0), m_d->canvasWidgetSize);

    QRectF rrect = irect & wrect;

    QRectF canvasBounds = reversedTransform.mapRect(rrect);
    QPointF offset = canvasBounds.topLeft();

    reversedTransform = reversedTransform * QTransform::fromTranslate(-offset.x(), -offset.y());

    m_d->widgetToViewport = reversedTransform;

    // qDebug() << "***********";
    // qDebug() << ppVar(m_d->flakeToPostprocessedFlake);
    // qDebug() << ppVar(m_d->postprocessedFlakeToWidget);
    // qDebug() << ppVar(m_d->widgetToViewport);
    // qDebug() << ppVar(documentOrigin());
    // qDebug() << ppVar(irect) << ppVar(wrect) << ppVar(rrect);
    // qDebug() << ppVar(canvasBounds);
    // qDebug() << ppVar(m_d->widgetToViewport.mapRect(irect));
}


KisCoordinatesConverter::KisCoordinatesConverter(KoViewConverter *viewConverter)
    : m_d(new Private)
{
    m_d->viewConverter = viewConverter;
}

KisCoordinatesConverter::~KisCoordinatesConverter()
{
    delete m_d;
}

void KisCoordinatesConverter::notifyZoomChanged()
{
    recalculateTransformations();
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

void KisCoordinatesConverter::setPostprocessingTransform(const QTransform &transform)
{
    m_d->postprocessingTransform = transform;
    recalculateTransformations();
}

QTransform KisCoordinatesConverter::postprocessingTransform() const
{
    return m_d->postprocessingTransform;
}


// see a comment in header file
DEFINE_TRANSFORM_METHODS(imageToDocument, documentToImage,
                         m_d->imageToDocument);

DEFINE_TRANSFORM_METHODS(documentToWidget, widgetToDocument,
                         m_d->documentToFlake * m_d->flakeToPostprocessedFlake
                         * m_d->postprocessedFlakeToWidget);

DEFINE_TRANSFORM_METHODS(widgetToViewport, viewportToWidget,
                         m_d->widgetToViewport);

DEFINE_TRANSFORM_METHODS(imageToViewport, viewportToImage,
                         m_d->imageToDocument * m_d->documentToFlake
                         * m_d->flakeToPostprocessedFlake * m_d->postprocessedFlakeToWidget
                         * m_d->widgetToViewport);

QTransform KisCoordinatesConverter::imageToWidgetTransform() const
{
    return m_d->imageToDocument * m_d->documentToFlake
        * m_d->flakeToPostprocessedFlake * m_d->postprocessedFlakeToWidget;
}

QTransform KisCoordinatesConverter::documentToWidgetTransform() const
{
    return m_d->documentToFlake
        * m_d->flakeToPostprocessedFlake * m_d->postprocessedFlakeToWidget;
}

QTransform KisCoordinatesConverter::flakeToWidgetTransform() const
{
    return m_d->flakeToPostprocessedFlake * m_d->postprocessedFlakeToWidget;
}

QTransform KisCoordinatesConverter::viewportToWidgetTransform() const
{
    return m_d->widgetToViewport.inverted();
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
        *polygon = viewportToWidgetTransform().map(imageRectInViewportPixels());
        *brushOrigin = QPoint(0,0);
    }
}

void KisCoordinatesConverter::getOpenGLCheckersInfo(QTransform *textureTransform,
                                                    QTransform *modelTransform,
                                                    QRectF *textureRect,
                                                    QRectF *modelRect)
{
    KisConfig cfg;
    QRectF viewportRect = imageRectInViewportPixels();

    *textureTransform = cfg.scrollCheckers() ? QTransform() :
        m_d->widgetToViewport.inverted() * m_d->postprocessedFlakeToWidget;

    *modelTransform = viewportToWidgetTransform();;
    *modelRect = viewportRect;
    *textureRect = QRectF(0, 0, viewportRect.width(),viewportRect.height());
}


// these functions return a bounding rect if the canvas is rotated

QRectF KisCoordinatesConverter::imageRectInWidgetPixels() const
{
    if(!m_d->image) return QRectF();
    return viewportToWidget(imageToViewport(m_d->image->bounds()));
}

QRectF KisCoordinatesConverter::imageRectInViewportPixels() const
{
    if(!m_d->image) return QRectF();
    return imageToViewport(m_d->image->bounds());
}

QSizeF KisCoordinatesConverter::imageSizeInFlakePixels() const
{
    if(!m_d->image) return QSizeF();

    qreal scaleX, scaleY;
    imageScale(&scaleX, &scaleY);
    QSize imageSize = m_d->image->size();

    return QSizeF(imageSize.width() * scaleX, imageSize.height() * scaleY);
}

QRectF KisCoordinatesConverter::widgetRectInFlakePixels() const
{
    // NOTE: this duplicates part of recalculateTransformations()
    QTransform reversedTransform = (m_d->flakeToPostprocessedFlake * m_d->postprocessedFlakeToWidget).inverted();
    return reversedTransform.mapRect(QRectF(QPoint(0,0), m_d->canvasWidgetSize));
}

QPoint KisCoordinatesConverter::offsetFromFlakeCenterPoint(const QPointF &pt) const
{
    QTransform transform = m_d->flakeToPostprocessedFlake * m_d->postprocessedFlakeToWidget;
    QPointF newWidgetCenter = transform.map(pt);
    QPointF oldWidgetCenter = QPointF(m_d->canvasWidgetSize.width() / 2, m_d->canvasWidgetSize.height() / 2);

    return documentOffset() + (newWidgetCenter - oldWidgetCenter).toPoint();
}

void KisCoordinatesConverter::imageScale(qreal *scaleX, qreal *scaleY) const
{
    // get the x and y zoom level of the canvas
    qreal zoomX, zoomY;
    m_d->viewConverter->zoom(&zoomX, &zoomY);

    // Get the KisImage resolution
    qreal resX = m_d->image->xRes();
    qreal resY = m_d->image->yRes();

    // Compute the scale factors
    *scaleX = zoomX / resX;
    *scaleY = zoomY / resY;
}
