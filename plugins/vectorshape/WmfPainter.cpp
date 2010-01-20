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

WmfPainter::WmfPainter()
    : KoWmfPaint()
{
}


//-----------------------------------------------------------------------------
// Virtual Painter

bool WmfPainter::begin()
{
    bool ret = true;
    if (mIsInternalPainter)
        ret = mPainter->begin(mTarget);

    // NOTE: NO SETUP OF ANY MATRIX STUFF IN HERE.

    mPainter->setBrush(QBrush(Qt::NoBrush));
    //mPainter->setBrush(QBrush(Qt::white));

    //kDebug(31000) << "Using QPainter: " << mPainter->pen() << mPainter->brush() 
    //              << "Background: " << mPainter->background() << " " << mPainter->backgroundMode();

    return ret;
}




// ---------------------------------------------------------------------


void WmfPainter::setWindowOrg(int /*left*/, int /*top*/)
{
}


void WmfPainter::setWindowExt(int /*w*/, int /*h*/)
{
}
