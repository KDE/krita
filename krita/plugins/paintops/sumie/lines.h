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
#ifndef _LINES_H
#define _LINES_H


#include <KoColor.h>
#include "kis_paint_device.h"

/*
* Some line algorithms and custom-made line algorithms
*/
class Lines{

public:
	Lines(){}
	~Lines(){}
	/// calls drawThickLine with thickness 1,1
	void drawLine(KisPaintDeviceSP dev, int x0,int y0,int x1,int y1,const KoColor &color);

	/// paints DDA line with thickness of 1px
	void drawDDALine(KisPaintDeviceSP image, int x1, int y1, int x2, int y2,const KoColor &color);
	/// custom made line, somehow irregular, it was a test to make it anti*aliased, but now it is quite nice effect
	void drawDDAALine(KisPaintDeviceSP image, int x1, int y1, int x2, int y2,const KoColor &color);
	/// draws anti-aliased line with thickness of 1px
	void drawWuLine(KisPaintDeviceSP dev, float x1, float y1, float x2, float y2, const KoColor &color);
	/// draws anti-aliased line with variable thickness for both end-points, the support for gradient color of line is not supported
	void drawThickLine(KisPaintDeviceSP dev, int x0, int y0, int x1, int y1,const KoColor color1, const KoColor color2, int w1, int w2);

	void drawThick(KisPaintDeviceSP dev,KoColor color1,const QPointF & start, const QPointF & end, int startWidth, int endWidth);

};

#endif
