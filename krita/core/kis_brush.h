/*
 *  kis_brush.h - part of Krayon
 *
 *  Copyright (c) 1999 Matthias Elter  <me@kde.org>
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
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef __kis_brush_h__
#define __kis_brush_h__

#include <qsize.h>
#include "kis_krayon.h"
#include "iconitem.h"

class QPoint;
class QPixmap;

class KisBrush : public KisKrayon
{
 public:
    KisBrush(QString file, bool monochrome = false, bool special = false);
    virtual ~KisBrush();

    void      setSpacing(int s) { m_spacing = s; }
    int       spacing() const { return m_spacing; }
    bool      isValid() const { return m_valid; }
    void      setHotSpot(QPoint);
    QPoint    hotSpot() const { return m_hotSpot; }

    uchar     value(int x, int y) const;
    uchar*    scanline(int) const;
    uchar*    bits() const;
   
    void      dump() const;

 private:
    void      loadViaQImage(QString file, bool monochrome = false);
    void      readBrushInfo(QString file);

    bool      m_valid;
    int       m_spacing;
    QPoint    m_hotSpot;
    uchar*    m_pData;
};

#endif

