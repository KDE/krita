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

#include <QTransform>

#include <KoViewConverter.h>

#include <kis_config.h>
#include <kis_image.h>


struct KisCoordinatesConverter::Private {
    KisImageWSP image;
    KoViewConverter *viewConverter;
    QPoint documentOffset;
    QPoint documentOrigin;
};


KisCoordinatesConverter::KisCoordinatesConverter(KoViewConverter *viewConverter)
    : m_d(new Private)
{
    m_d->viewConverter = viewConverter;
}

KisCoordinatesConverter::~KisCoordinatesConverter()
{
    delete m_d;
}

void KisCoordinatesConverter::setImage(KisImageWSP image)
{
    m_d->image = image;
}

void KisCoordinatesConverter::setDocumentOrigin(const QPoint &origin)
{
    m_d->documentOrigin = origin;
}

void KisCoordinatesConverter::setDocumentOffset(const QPoint &offset)
{
    m_d->documentOffset = offset;
}

QPoint KisCoordinatesConverter::documentOrigin() const
{
    return m_d->documentOrigin;
}

QPoint KisCoordinatesConverter::documentOffset() const
{
    return m_d->documentOffset;
}

QPointF KisCoordinatesConverter::imageToViewport(const QPointF &pt) const
{
    QPointF docPt = m_d->image->pixelToDocument(pt);
    QPointF tempPt = m_d->viewConverter->documentToView(docPt);

    return tempPt - m_d->documentOffset;
}

QPointF KisCoordinatesConverter::viewportToImage(const QPointF &pt) const
{
    QPointF tempPt = pt + m_d->documentOffset;
    QPointF docPt = m_d->viewConverter->viewToDocument(tempPt);

    return m_d->image->documentToPixel(docPt);
}

DEFINE_RECT_METHOD(imageToViewport);
DEFINE_RECT_METHOD(viewportToImage);

QPointF KisCoordinatesConverter::widgetToViewport(const QPointF &pt) const
{
    return pt - m_d->documentOrigin;
}

QPointF KisCoordinatesConverter::viewportToWidget(const QPointF &pt) const
{
    return pt + m_d->documentOrigin;
}

DEFINE_RECT_METHOD(widgetToViewport);
DEFINE_RECT_METHOD(viewportToWidget);


QPointF KisCoordinatesConverter::widgetToDocument(const QPointF &pt) const
{
    QPointF tempPt = pt - m_d->documentOrigin + m_d->documentOffset;
    return m_d->viewConverter->viewToDocument(tempPt);
}

QPointF KisCoordinatesConverter::documentToWidget(const QPointF &pt) const
{
    QPointF tempPt = m_d->viewConverter->documentToView(pt);
    return tempPt + m_d->documentOrigin - m_d->documentOffset;
}

DEFINE_RECT_METHOD(widgetToDocument);
DEFINE_RECT_METHOD(documentToWidget);

QPointF KisCoordinatesConverter::imageToDocument(const QPointF &pt) const
{
    return m_d->image->pixelToDocument(pt);
}

QPointF KisCoordinatesConverter::documentToImage(const QPointF &pt) const
{
    return m_d->image->documentToPixel(pt);
}

DEFINE_RECT_METHOD(imageToDocument);
DEFINE_RECT_METHOD(documentToImage);

QTransform KisCoordinatesConverter::imageToWidgetTransform() const
{
    QTransform transform;

    qreal scaleX, scaleY;
    imageScale(&scaleX, &scaleY);

    transform.scale(scaleX, scaleY);
    transform.translate(-m_d->documentOffset.x() / scaleX + m_d->documentOrigin.x() / scaleX,
                        -m_d->documentOffset.y() / scaleY + m_d->documentOrigin.y() / scaleY);

    return transform;
}

QTransform KisCoordinatesConverter::documentToWidgetTransform() const
{
    QTransform transform;

    qreal zoomX, zoomY;
    m_d->viewConverter->zoom(&zoomX, &zoomY);

    transform.scale(zoomX, zoomY);
    transform.translate(-m_d->documentOffset.x() / zoomX + m_d->documentOrigin.x() / zoomX,
                        -m_d->documentOffset.y() / zoomY + m_d->documentOrigin.y() / zoomY);

    return transform;
}

QTransform KisCoordinatesConverter::flakeToWidgetTransform() const
{
    QTransform transform;
    transform.translate(-m_d->documentOffset.x() + m_d->documentOrigin.x(),
                        -m_d->documentOffset.y() + m_d->documentOrigin.y());

    return transform;
}

QTransform KisCoordinatesConverter::viewportToWidgetTransform() const
{
    QTransform transform;
    transform.translate(m_d->documentOrigin.x(), m_d->documentOrigin.y());

    return transform;
}

QTransform KisCoordinatesConverter::checkersToWidgetTransform() const
{
    QTransform transform;
    transform.translate(m_d->documentOrigin.x(), m_d->documentOrigin.y());

    KisConfig cfg;
    if (!cfg.scrollCheckers())
            transform.translate(+m_d->documentOffset.x(), +m_d->documentOffset.y());

    return transform;
}

QSize KisCoordinatesConverter::imageSizeInWidgetPixels() const
{
    return imageRectInWidgetPixels().toAlignedRect().size();
}

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
