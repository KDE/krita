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


#include <KoViewConverter.h>
#include <kis_image.h>


struct KisCoordinatesConverter::Private {
    KisImageWSP image;
    KoViewConverter *viewConverter;
    QPoint documentOffset;
    QPoint documentOrigin;
};


KisCoordinatesConverter::KisCoordinatesConverter(KisImageWSP image, KoViewConverter *viewConverter)
    : m_d(new Private)
{
    m_d->image = image;
    m_d->viewConverter = viewConverter;
}

KisCoordinatesConverter::~KisCoordinatesConverter()
{
    delete m_d;
}

void KisCoordinatesConverter::setDocumentOrigin(const QPoint &origin)
{
    m_d->documentOrigin = origin;
}

void KisCoordinatesConverter::setDocumentOffset(const QPoint &offset)
{
    m_d->documentOffset = offset;
}

QRectF KisCoordinatesConverter::imageToViewport(const QRect &imageRect)
{
    QRectF docRect = m_d->image->pixelToDocument(QRectF(imageRect));
    QRectF tempRect = m_d->viewConverter->documentToView(docRect);

    return tempRect.translated(-m_d->documentOffset);
}

QRect KisCoordinatesConverter::viewportToImage(const QRectF &viewportRect)
{
    QRectF tempRect = viewportRect.translated(m_d->documentOffset);
    QRectF docRect = m_d->viewConverter->viewToDocument(tempRect);

    // FIXME: intersection?
    return m_d->image->documentToPixel(docRect).toAlignedRect().intersected(m_d->image->bounds());
}

QRectF KisCoordinatesConverter::imageToWidget(const QRect &imageRect)
{
    return viewportToWidget(imageToViewport(imageRect));
}

QRect KisCoordinatesConverter::widgetToImage(const QRectF &widgetRect)
{
    return viewportToImage(widgetToViewport(widgetRect));
}

QRectF KisCoordinatesConverter::widgetToViewport(const QRectF &widgetRect)
{
    return widgetRect.translated(-(m_d->documentOrigin + m_d->documentOffset));
}

QRectF KisCoordinatesConverter::viewportToWidget(const QRectF &viewportRect)
{
    return viewportRect.translated(m_d->documentOrigin + m_d->documentOffset);
}

void KisCoordinatesConverter::imageScale(qreal *scaleX, qreal *scaleY)
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
