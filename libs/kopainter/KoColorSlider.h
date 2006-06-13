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

#ifndef KO_COLORSLIDER_H
#define KO_COLORSLIDER_H

#include <kselector.h>
#include <koffice_export.h>

#include "KoColor.h"

class KOPAINTER_EXPORT KoColorSlider : public KSelector
{
  Q_OBJECT
public:
  KoColorSlider(KoColorSpace* colorSpace, QWidget *parent = 0);
  KoColorSlider(KoColorSpace* colorSpace, Qt::Orientation o, QWidget *parent = 0);
  virtual ~KoColorSlider();

public:
  void setColors( const KoColor& minColor, const KoColor& maxColor);

protected:
  virtual void drawContents( QPainter* );

protected:
  KoColor m_colors[2];

  KoColorSpace * m_colorSpace;
};

#endif
