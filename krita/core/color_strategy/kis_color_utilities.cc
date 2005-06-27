/*
 *  Copyright (c) 2005 Boudewijn Rempt <boud@valdyas.org>
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
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */
#include <qcolor.h>
#include <qglobal.h>

#include "kis_color_conversions.h"
#include "kis_color_utilities.h"


Q_UINT8 matchColors(const QColor & c, const QColor & c2, Q_UINT8 fuzziness)
{
#if 1
	// XXX: Is it enough to compare just hue, or should we compare saturation
	// and value too?
	int h1, s1, v1, h2, s2, v2;
	rgb_to_hsv(c.red(), c.green(), c.blue(), &h1, &s1, &v1);
	rgb_to_hsv(c2.red(), c2.green(), c2.blue(), &h2, &s2, &v2);

	int diff = QMAX(QABS(v1 - v2), QMAX(QABS(s1 - s2), QABS(h1 - h2)));

	if (diff > fuzziness) return 0;

	if (diff == 0) return 255;

	return 255 - (diff / fuzziness * 255);
#else //XXX: See doc/colordiff for this formulate. I don't know how to map the long values to a 0-255 range.
	long r,g,b;
	long rmean;

	rmean = ( (int)c.red() + (int)c2.red() ) / 2;

	r = (int)c.red() - (int)c2.red();
	g = (int)c.green() - (int)c2.green();
	b = (int)c.blue() - (int)c2.blue();

	long diff = (((512+rmean)*r*r)>>8) + 4*g*g + (((767-rmean)*b*b)>>8);

        if (diff > fuzziness) return 0;

        if (diff == 0,0) return 255;

	return (Q_UINT8) diff;

#endif
}

// XXX: Poynton says: hsv/hls is not what one ought to use for colour calculations.
//      Unfortunately, I don't know enough to be able to use anything else.

bool isReddish(int h)
{
	return ((h > 330 && h < 360) || ( h > 0 && h < 40));
}

bool isYellowish(int h)
{
	return (h> 40 && h < 65);
}

bool isGreenish(int h)
{
	return (h > 70 && h < 155);
}

bool isCyanish(int h)
{
	return (h > 150 && h < 190);
}

bool isBlueish(int h)
{
	return (h > 185 && h < 270);
}

bool isMagentaish(int h)
{
	return (h > 265 && h < 330);
}

bool isHighlight(int v)
{
	return (v > 200);
}

bool isMidTone(int v)
{
	return (v > 100 && v < 200);
}

bool isShadow(int v)
{
	return (v < 100);
}

