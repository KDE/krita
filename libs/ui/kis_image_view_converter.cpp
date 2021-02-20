/*
 *  SPDX-FileCopyrightText: 2007 Thomas Zander <zander@kde.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
