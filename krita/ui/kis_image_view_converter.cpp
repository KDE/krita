/*
 *  Copyright (c) 2007 Thomas Zander <zander@kde.org>
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

#include "kis_image_view_converter.h"

KisImageViewConverter::KisImageViewConverter(const KisImageWSP image)
        : m_image(image)
{
    Q_ASSERT(image);
}

// remember here; document is postscript points;  view is krita pixels.

QPointF KisImageViewConverter::documentToView(const QPointF &documentPoint) const
{
    return QPointF(documentToViewX(documentPoint.x()), documentToViewX(documentPoint.y()));
}

QPointF KisImageViewConverter::viewToDocument(const QPointF &viewPoint) const
{
    return QPointF(viewToDocumentX(viewPoint.x()), viewToDocumentY(viewPoint.y()));
}

QRectF KisImageViewConverter::documentToView(const QRectF &documentRect) const
{
    return QRectF(documentToView(documentRect.topLeft()),
                  QSizeF(documentToViewX(documentRect.width()), documentToViewY(documentRect.height())));
}

QRectF KisImageViewConverter::viewToDocument(const QRectF &viewRect) const
{
    return QRectF(viewToDocument(viewRect.topLeft()),
                  QSizeF(viewToDocumentX(viewRect.width()), viewToDocumentY(viewRect.height())));
}

QSizeF KisImageViewConverter::documentToView(const QSizeF &documentSize) const
{
    return QSizeF(documentToViewX(documentSize.width()), documentToViewY(documentSize.height()));
}

QSizeF KisImageViewConverter::viewToDocument(const QSizeF &viewSize) const
{
    return QSizeF(viewToDocumentX(viewSize.width()), viewToDocumentY(viewSize.height()));
}

void KisImageViewConverter::zoom(qreal *zoomX, qreal *zoomY) const
{
    Q_ASSERT(zoomX);
    Q_ASSERT(zoomY);
    *zoomX = m_image->xRes();
    *zoomY = m_image->yRes();
}
