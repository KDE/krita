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

#ifndef _WMFPAINTER_H_
#define _WMFPAINTER_H_

#include "kowmfpaint.h"

#include <QtGui/QPainter>

class QPolygon;

/**
 * WmfPainter inherits the abstract class KoWmfPaint and does away
 * with all the QPainter window stuff since we're already taking care
 * of that in the calling code.
 */

class WmfPainter: public KoWmfPaint
{
public:
    WmfPainter();
    ~WmfPainter() { }

private:
    // -------------------------------------------------------------------------
    // Reimplemented functions

    bool  begin();

    void  setWindowOrg(int left, int top);
    void  setWindowExt(int width, int height);
};

#endif
