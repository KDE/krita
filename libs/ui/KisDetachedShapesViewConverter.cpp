/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisDetachedShapesViewConverter.h"

KisDetachedShapesViewConverter::KisDetachedShapesViewConverter(qreal xRes, qreal yRes)
    : m_xRes(xRes),
      m_yRes(yRes)
{
    setZoom(0.1); // set the superclass to not hit the optimization of zoom=100%
}

KisClonableViewConverter *KisDetachedShapesViewConverter::clone() const
{
    return new KisDetachedShapesViewConverter(m_xRes, m_yRes);
}

void KisDetachedShapesViewConverter::zoom(qreal *zoomX, qreal *zoomY) const
{
    Q_ASSERT(zoomX);
    Q_ASSERT(zoomY);
    *zoomX = m_xRes;
    *zoomY = m_yRes;
}

qreal KisDetachedShapesViewConverter::documentToViewX(qreal documentX) const
{
    return documentX * m_xRes;
}

qreal KisDetachedShapesViewConverter::documentToViewY(qreal documentY) const
{
    return documentY * m_yRes;
}

qreal KisDetachedShapesViewConverter::viewToDocumentX(qreal viewX) const
{
    return viewX / m_xRes;
}

qreal KisDetachedShapesViewConverter::viewToDocumentY(qreal viewY) const
{
    return viewY / m_yRes;
}

qreal KisDetachedShapesViewConverter::zoom() const
{
    Q_ASSERT_X(0, "KisDetachedShapesViewConverter::zoom()",
               "Not possible to return a single zoom. "
               "Don't use it. Sorry.");

    return m_xRes;
}
