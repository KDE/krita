/* This file is part of the KDE libraries
   Copyright (c) 1998 Stefan Taferner
                 2001/2003 thierry lorthiois (lorthioist@wanadoo.fr)
                 2011 Inge Wallin (inge@lysator.liu.se)

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include <kdebug.h>

#include "WmfStack.h"
#include "WmfAbstractBackend.h"
#include "WmfDeviceContext.h"

/**
   Namespace for Windows Metafile (WMF) classes
*/
namespace Libwmf
{

void KoWmfBrushHandle::apply(WmfDeviceContext *dc)
{
    dc->brush = brush;
    dc->changedItems |= DCBrush;
}

void KoWmfPenHandle::apply(WmfDeviceContext *dc)
{
    kDebug(31000) << "Setting pen" << pen;
    dc->pen = pen;
    dc->changedItems |= DCPen;
}

void KoWmfPatternBrushHandle::apply(WmfDeviceContext *dc)
{
    dc->brush = brush;
    dc->changedItems |= DCBrush;
}

void KoWmfFontHandle::apply(WmfDeviceContext *dc)
{
    dc->font = font;
    dc->escapement = escapement;
    dc->orientation = orientation;
    dc->height = height;
    dc->changedItems |= DCFont; // Includes the font itself, the rotation and the height;
}

}
