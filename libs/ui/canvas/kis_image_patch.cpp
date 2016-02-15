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

#include "kis_image_patch.h"

#include <QPainter>
#include "kis_debug.h"

#include <math.h>

/****** Some helper functions *******/

inline void scaleRect(QRectF &rc, qreal scaleX, qreal scaleY)
{
    qreal x, y, w, h;
    rc.getRect(&x, &y, &w, &h);

    x *= scaleX;
    y *= scaleY;
    w *= scaleX;
    h *= scaleY;

    rc.setRect(x, y, w, h);
}

inline void scaleRect(QRect &rc, qreal scaleX, qreal scaleY)
{
    qint32 x, y, w, h;
    rc.getRect(&x, &y, &w, &h);

    x *= scaleX;
    y *= scaleY;
    w *= scaleX;
    h *= scaleY;

    rc.setRect(x, y, w, h);
}

/*********** KisImagePatch ************/

KisImagePatch::KisImagePatch()
    : m_scaleX(0), m_scaleY(0)
{
}

KisImagePatch::KisImagePatch(QRect imageRect, qint32 borderWidth,
                             qreal scaleX, qreal scaleY)
    : m_scaleX(scaleX)
    , m_scaleY(scaleY)
    , m_isScaled(false)
{
    // First we get unscaled rects
    m_interestRect = QRectF(borderWidth, borderWidth,
                            imageRect.width(), imageRect.height());
    m_patchRect = imageRect.adjusted(-borderWidth, -borderWidth,
                                     borderWidth, borderWidth);
    // And then we scale them
    scaleRect(m_interestRect, scaleX, scaleY);
    scaleRect(m_patchRect, scaleX, scaleY);

    dbgRender << "A new patch has been created:";
    dbgRender << ppVar(scaleX) << ppVar(scaleY);
    dbgRender << ppVar(m_interestRect);
    dbgRender << ppVar(m_patchRect);
}

void KisImagePatch::setImage(QImage image)
{
    m_image = image;
    m_isScaled = false;
}

void KisImagePatch::preScale(const QRectF &dstRect)
{
    if (m_isScaled) return;

    qreal scaleX = dstRect.width() / m_interestRect.width();
    qreal scaleY = dstRect.height() / m_interestRect.height();

    QSize newImageSize = QSize(ceil(m_image.width() * scaleX),
                                   ceil(m_image.height() * scaleY));
    // Calculating new _aligned_ scale
    scaleX = qreal(newImageSize.width()) / m_image.width();
    scaleY = qreal(newImageSize.height()) / m_image.height();

    m_scaleX *= scaleX;
    m_scaleY *= scaleY;

    scaleRect(m_interestRect, scaleX, scaleY);

    m_image = m_image.scaled(newImageSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

    m_isScaled = true;

}

QRect KisImagePatch::patchRect()
{
    return m_patchRect;
}

bool KisImagePatch::isValid()
{
    return !m_image.isNull();
}

void KisImagePatch::drawMe(QPainter &gc,
                           const QRectF &dstRect,
                           QPainter::RenderHints renderHints)
{
    gc.save();
    gc.setCompositionMode(QPainter::CompositionMode_Source);
    gc.setRenderHints(renderHints, true);
    gc.drawImage(dstRect, m_image, m_interestRect);
    gc.restore();

#if 0
    /**
     * Just for debugging purposes
     */
    qreal scaleX = dstRect.width() / m_interestRect.width();
    qreal scaleY = dstRect.height() / m_interestRect.height();
    dbgRender << "## PATCH.DRAWME #####################";
    dbgRender << ppVar(scaleX) << ppVar(scaleY);
    dbgRender << ppVar(m_patchRect);
    dbgRender << ppVar(m_interestRect);
    dbgRender << ppVar(dstRect);
    dbgRender << "## EODM #############################";
#endif
}
