/*
 *  Copyright (c) 2008 Lukas Tvrdy <LukasT.dev@gmail.com>
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

#include <QImage>

using namespace std;

class Stroke {

public:
	void drawWuLine(QImage *dev, float x1, float y1, float x2, float y2, float width, const QColor &color);
	void drawGSLine(QImage *image, int x0, int y0, int x1, int y1, int w1, int w2, const QColor &color);
	void drawDDALine(QImage *image, int x1, int y1, int x2, int y2, const QColor &color);


private:
	int inline trunc(float value);
	float inline frac(float value);
	float inline invertFrac(float value);
	int MapBrightness(float p, int thickness);
	int gsfilter(float val);

};

#endif
