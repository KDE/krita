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

#include "kis_point.h"

class KisRect
{
public:
	KisRect();
	KisRect(double x, double y, double w, double h);
	KisRect(const QRect& qr);

	void setX(double x);
	void setY(double y);
	void setWidth(double w);
	void setHeight(double h);

	double x() const;
	double y() const;
	double width() const;
	double height() const;

	bool isNull() const;

	QRect qRect() const;

private:
	double m_x1;
	double m_y1;
	double m_x2;
	double m_y2;
};

inline
KisRect::KisRect()
{
	m_x1 = 0;
	m_y1 = 0;
	m_x2 = 0;
	m_y2 = 0;
}

inline
KisRect::KisRect(double x, double y, double w, double h)
{
	m_x1 = x;
	m_y1 = y;
	m_x2 = x + w;
	m_y2 = y + h;
}

inline
KisRect::KisRect(const QRect& qr)
{
	m_x1 = qr.x();
	m_y1 = qr.y();
	m_x2 = qr.x() + qr.width();
	m_y2 = qr.y() + qr.height();
}

inline
void KisRect::setX(double x)
{
	m_x1 = x;
}

inline
void KisRect::setY(double y)
{
	m_y1 = y;
}

inline
void KisRect::setWidth(double w)
{
	m_x2 = m_x1 + w;
}

inline
void KisRect::setHeight(double h)
{
	m_y2 = m_y1 + h;
}

inline
double KisRect::x() const
{
	return m_x1;
}

inline
double KisRect::y() const
{
	return m_y1;
}

inline
double KisRect::width() const
{
	return m_x2 - m_x1;
}

inline
double KisRect::height() const
{
	return m_y2 - m_y1;
}

#endif // KIS_RECT_H_

