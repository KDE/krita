/*
 *  kis_pattern.h - part of Krayon
 *
 *  Copyright (c) 2000 Matthias Elter  <elter@kde.org>
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

#ifndef __kis_pattern_h__
#define __kis_pattern_h__

#include <qsize.h>

#include "iconitem.h"
#include "kis_krayon.h"

class QPoint;
class QPixmap;
class QImage;

class KisPattern : public KisKrayon
{
public:
    KisPattern(QString file);
    KisPattern(int formula);
    virtual ~KisPattern();

    bool isValid()	const { return m_valid; }
    void setSpacing(int s) { m_spacing = s; }
    int  spacing() const { return m_spacing; }
    QPoint hotSpot() const { return m_hotSpot; }
    bool tileSymmetric() const { return m_TileSymmetric; }
    
private:
    void loadViaQImage(QString file);
    void loadViaFormula(int formula);
    void readPatternInfo(QString file);

    QPoint m_hotSpot;
    
    bool m_valid;
    bool m_TileSymmetric;            

    int  m_spacing;
    int  m_TileWidth;
    int  m_TileHeight;

};

#endif

