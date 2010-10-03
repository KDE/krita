/* This file is part of the KDE libraries
 *
 * Copyright (c) 2009 Inge Wallin (inge@lysator.liu.se)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License version 2 as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include "WmfPainter.h"

#include <QPolygon>
#include <QPrinter>

#include <kdebug.h>

#include "VectorShape.h"  // For DEBUG_VECTORSHAPE


WmfPainter::WmfPainter()
    : KoWmfPaint()
{
    mScaleX = 1.0;
    mScaleY = 1.0;
}


// ---------------------------------------------------------------------


// We use our own setWindowExt that are basically noops because the
// shape has already set up the mapping between logical coordinates
// within the WMF and the size of the shape.
//
// The only exception from this is that we may have to flip coordinate
// systems in setWindowExt if the width and/or the height is negative.


void WmfPainter::setWindowExt(int width, int height)
{
    mExtWidth  = width;
    mExtHeight = height;

    // Unscale the scale done last time (the first time, this code is a noop).
    qreal dx = mInternalWorldMatrix.dx();
    qreal dy = mInternalWorldMatrix.dy();
    mInternalWorldMatrix.translate(-dx, -dy);
    mPainter->translate(-dx, -dy);
    mInternalWorldMatrix.scale(1.0 / mScaleX, 1.0 / mScaleY);
    mPainter->scale(1.0 / mScaleX, 1.0 / mScaleY);

    // Flip the wmf if necessary.
    mScaleX = (width < 0)  ? -1.0 : 1.0;
    mScaleY = (height < 0) ? -1.0 : 1.0;

#if DEBUG_VECTORSHAPE
    kDebug(31000) << "Origin =" << mOrgX << mOrgY;
    kDebug(31000) << "size   =" << width << height;
#endif

    // Now scale to the new values (=flip), and we want to flip in place.
    qreal dx2 = (mOrgX + mOrgX + width) / 2.0;
    qreal dy2 = (mOrgY + mOrgY + height) / 2.0;
    mInternalWorldMatrix.translate(dx2, dy2);
    mInternalWorldMatrix.scale(mScaleX, mScaleY);
    mInternalWorldMatrix.translate(-dx2, -dy2);
    mPainter->translate(dx2, dy2);
    mPainter->scale(mScaleX, mScaleY);
    mPainter->translate(-dx2, -dy2);

#if 0
    // Debug code.  Draw a rectangle with some decoration to show see
    // if all the transformations work.
    mPainter->save();

    mPainter->setPen(Qt::black);
    QRect windowRect = QRect(QPoint(mOrgX, mOrgY),
                             QSize(mExtWidth, mExtHeight));
    mPainter->drawRect(windowRect);
    mPainter->drawLine(QPoint(mOrgX, mOrgY), QPoint(0, 0));

    mPainter->setPen(Qt::red);
    mPainter->drawRect(boundingRect());

    mPainter->drawLine(boundingRect().topLeft(), QPoint(0, 0));
    mPainter->restore();

    kDebug(31000) << "Window rect: " << windowRect;
    kDebug(31000) << "Bounding rect: " << boundingRect();
#endif
}
