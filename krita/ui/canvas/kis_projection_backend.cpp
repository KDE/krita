/*
 *  Copyright (c) 2009 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_projection_backend.h"

#include <QRect>
#include <QRectF>
#include <QPainter>
#include "kis_debug.h"

#include <math.h>
#include <qimageblitz.h>

KisProjectionBackend::~KisProjectionBackend()
{
}

void KisProjectionBackend::alignSourceRect(QRect& rect, qreal scale)
{
    Q_UNUSED(rect);
    Q_UNUSED(scale);
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
}

#define ceiledSize(sz) QSize(ceil((sz).width()), ceil((sz).height()))

void KisImagePatch::prescaleWithBlitz(QRectF dstRect)
{
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

    m_image = Blitz::smoothScale(m_image,
                                 newImageSize,
                                 Qt::IgnoreAspectRatio);

    dbgRender << "## PATCH.PRESCALEBLITZ ############";
    dbgRender << ppVar(scaleX) << ppVar(scaleY);
    dbgRender << ppVar(newImageSize);
    dbgRender << "## EOB ############################";
}
