/*
 *  Copyright (c) 2007 Emanuele Tamponi <emanuele@valinor.it>
 *
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

#ifndef COLORSPOT_H_
#define COLORSPOT_H_

#include <QToolButton>

class KoColor;
class QColor;

class ColorSpot : public QToolButton {
	Q_OBJECT

	typedef QToolButton super;

	public:
		ColorSpot (QWidget *parent, const KoColor &color);

		~ColorSpot();

		void setColor(const KoColor &color);

		KoColor color() const { return m_color; }

	private:
		KoColor m_color;
};


#endif // COLORSPOT_H_
