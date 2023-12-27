/*
 *  SPDX-FileCopyrightText: 2007 Thomas Zander <zander@kde.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_image_view_converter.h"
#include "kis_image.h"
#include "kis_pointer_utils.h"

KisImageViewConverter::KisImageViewConverter()
    : KisImageViewConverter(KisImageSP())
{
}

KisImageViewConverter::KisImageViewConverter(const KisImageWSP image)
    : KisImageViewConverter(toQShared(new KisImageResolutionProxy(image)))
{
}

KisImageViewConverter::KisImageViewConverter(KisImageResolutionProxySP proxy)
    : m_proxy(proxy)
{
    setZoom(0.1); // set the superclass to not hit the optimization of zoom=100%
}

KisImageViewConverter::KisImageViewConverter(const KisImageViewConverter &rhs)
    : KisClonableViewConverter(rhs)
    , m_proxy(rhs.m_proxy)
{
}

KisImageViewConverter::~KisImageViewConverter()
{
}

KisClonableViewConverter *KisImageViewConverter::clone() const
{
    return new KisImageViewConverter(*this);
}

void KisImageViewConverter::setImage(KisImageWSP image)
{
    m_proxy = m_proxy->createOrCloneDetached(image);
}

// remember here; document is postscript points;  view is krita pixels.

void KisImageViewConverter::zoom(qreal *zoomX, qreal *zoomY) const
{
    Q_ASSERT(zoomX);
    Q_ASSERT(zoomY);
    *zoomX = effectiveXRes();
    *zoomY = effectiveYRes();
}

/// convert from flake to krita units
qreal KisImageViewConverter::documentToViewX(qreal documentX) const {
    return documentX * effectiveXRes();
}

/// convert from flake to krita units
qreal KisImageViewConverter::documentToViewY(qreal documentY) const {
    return documentY * effectiveYRes();
}

/// convert from krita to flake units
qreal KisImageViewConverter::viewToDocumentX(qreal viewX) const {
    return viewX / effectiveXRes();
}

/// convert from krita to flake units
qreal KisImageViewConverter::viewToDocumentY(qreal viewY) const {
    return viewY / effectiveYRes();
}

qreal KisImageViewConverter::zoom() const
{
    Q_ASSERT_X(0, "KisImageViewConverter::zoom()",
               "Not possible to return a single zoom. "
               "Don't use it. Sorry.");

    return effectiveXRes();
}

qreal KisImageViewConverter::effectiveXRes() const
{
    return m_proxy->xRes();
}

qreal KisImageViewConverter::effectiveYRes() const
{
    return m_proxy->yRes();
}
