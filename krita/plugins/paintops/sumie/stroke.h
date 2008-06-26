/*
 *  Copyright (c) 2000 Clara Chan <x@unknown.com>
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

#ifndef STROKE_H
#define STROKE_H

#include <cmath>

#include <QVector>

#include <kis_types.h>
#include <KoColor.h>

class QRect;

class Stroke {

public:

    Stroke();
    virtual ~Stroke();

    void draw (KisPaintDeviceSP dev);
    void setColor ( const KoColor & c );
	double x1,y1,x2,y2;

private:
    void drawLine( KisPaintDeviceSP dev, double x1, double y1, double x2, double y2, double width, const KoColor & color );
    /*void drawWuLine(KisPaintDeviceSP dev, double x1, double y1, double x2, double y2, double width, const KoColor & color );*/
	void drawGSLine(KisPaintDeviceSP image, int x0, int y0, int x1, int y1, int w1, int w2, const QColor &color);
	int gsfilter(float val);    
    
private:
    KoColor m_color;
};

#endif
