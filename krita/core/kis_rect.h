/*
 *  Copyright (c) 2004 Adrian Page <adrian@pagenet.plus.com>
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
#if !defined KIS_RECT_H_
#define KIS_RECT_H_

#include <qrect.h>
#include <koRect.h>
#include "kis_point.h"

class KisRect : public KoRect
{
	typedef KoRect super;
public:
	KisRect() {}
	KisRect(double x, double y, double w, double h) : super(x, y, w, h) {}
	KisRect(const KisPoint& topLeft, const KisPoint& bottomRight) : super(topLeft, bottomRight) {}
	KisRect(const QRect& qr) : super(qr.x(), qr.y(), qr.width(), qr.height()) {}
	KisRect(const KoRect& r) : super(r) {}

	QRect qRect() const;

private:
	// Use qRect() which uses ceil() and floor() to return a rectangle 
	// 'enclosing' the rectangle, whereas toQRect rounds the points.
	QRect toQRect() const;
};

#endif // KIS_RECT_H_

