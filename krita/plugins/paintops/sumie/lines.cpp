/*
 *  Copyright (c) 2008 Lukas Tvrdy <lukast.dev@gmail.com>
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
#include "lines.h"
#include <KoColor.h>
#include <kis_paint_device.h>
#include <kis_painter.h>

void Lines::drawLine(KisPaintDeviceSP dev, int x0,int y0,int x1,int y1,const KoColor &color)
{
    KisPainter gc( dev );
    gc.setPaintColor( color );
    gc.drawLine( QPointF(x0, y0), QPointF(x1, y1) );
}


void Lines::drawDDALine(KisPaintDeviceSP image, int x0, int y0, int x1, int y1,const KoColor &color)
{
    KisPainter gc( image );
    gc.setPaintColor( color );
    gc.drawDDALine( QPointF(x0, y0), QPointF(x1, y1) );
}

void Lines::drawDDAALine(KisPaintDeviceSP image, int x0, int y0, int x1, int y1,const KoColor &color)
{
    KisPainter gc( image );
    gc.setPaintColor( color );
    gc.drawWobblyLine( QPointF(x0, y0), QPointF(x1, y1) );


}
void Lines::drawWuLine ( KisPaintDeviceSP dev, float x0, float y0, float x1, float y1, const KoColor &color )
{
    KisPainter gc( dev );
    gc.setPaintColor( color );
    gc.drawWuLine( QPointF(x0, y0), QPointF(x1, y1) );


}

// cool wu lines with thickness support
void Lines::drawThickLine ( KisPaintDeviceSP dev, int x0, int y0, int x1, int y1,
                            const KoColor color1, const KoColor color2, int w1, int w2 )
{
    Q_UNUSED(color1);
    KisPainter gc( dev );
    gc.setPaintColor( color1 );
    gc.drawThickLine( QPointF(x0, y0), QPointF(x1, y1), w1, w2);

}


