/**
 * Copyright (c) 2006 Casper Boemann (cbr@boemann.dk)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
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
#ifndef KOCOLORPATCH_H
#define KOCOLORPATCH_H

#include <QFrame>

#include <KoColor.h>

/**
 *  The small widget showing the selected color
 */
class KOPAINTER_EXPORT KoColorPatch : public QFrame
{
  Q_OBJECT
public:
  KoColorPatch( QWidget *parent );
  virtual ~KoColorPatch();

  void setColor( const KoColor c );

protected:
  virtual void paintEvent    ( QPaintEvent * pe );

private:
  KoColor m_color;
};

#endif
