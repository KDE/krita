/* This file is part of the KDE project
   Copyright (C) 2006 Sven Langkamp <longamp@reallygood.de>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#ifndef KO_XYCOLORSELECTOR_H
#define KO_XYCOLORSELECTOR_H

#include <kxyselector.h>

#include "KoColor.h"

class QPainter;
class KoColorSpace;

class KOPAINTER_EXPORT KoXYColorSelector : public KXYSelector
{
  Q_OBJECT

public:
    KoXYColorSelector( KoColorSpace* colorSpace,  QWidget *parent=0 );

    void setColors( const KoColor& topLeftColor, const KoColor& topRightColor,  const KoColor& bottomLeftColor, const KoColor& bottomRightColor);

protected:
    virtual void drawContents( QPainter *painter );

private:
    KoColor m_colors[4];
    enum {TOPLEFT, TOPRIGHT, BOTTOMLEFT, BOTTOMRIGHT};
    KoColorSpace* m_colorSpace;
};

#endif
