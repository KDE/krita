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

#include <QTransform>


KisImageViewConverter::KisImageViewConverter(const KisImageWSP image)
        : m_image(image)
{
    Q_ASSERT(image);
    setZoom(0.1); // set the superclass to not hit the optimization of zoom=100%
}

void KisImageViewConverter::setImage(KisImageWSP image)
{
    m_image = image;
}

QTransform KisImageViewConverter::documentToView() const
{
    return QTransform::fromScale(m_image->xRes(), m_image->yRes());
}

QTransform KisImageViewConverter::viewToDocument() const
{
    return QTransform::fromScale(1.0 / m_image->xRes(), 1.0 / m_image->yRes());
}

// remember here; document is postscript points;  view is krita pixels.

void KisImageViewConverter::zoom(qreal *zoomX, qreal *zoomY) const
{
    Q_ASSERT(zoomX);
    Q_ASSERT(zoomY);
    *zoomX = m_image->xRes();
    *zoomY = m_image->yRes();
}

/// convert from flake to krita units
qreal KisImageViewConverter::documentToViewX(qreal documentX) const {
    return documentX * m_image->xRes();
}

/// convert from flake to krita units
qreal KisImageViewConverter::documentToViewY(qreal documentY) const {
    return documentY * m_image->yRes();
}

/// convert from krita to flake units
qreal KisImageViewConverter::viewToDocumentX(qreal viewX) const {
    return viewX / m_image->xRes();
}

/// convert from krita to flake units
qreal KisImageViewConverter::viewToDocumentY(qreal viewY) const {
    return viewY / m_image->yRes();
}

qreal KisImageViewConverter::zoom() const
{
    Q_ASSERT_X(0, "KisImageViewConverter::zoom()",
               "Not possible to return a single zoom. "
               "Don't use it. Sorry.");

    return m_image->xRes();
}
