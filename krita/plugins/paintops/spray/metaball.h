/*
 *  Copyright (c) 2009 Lukas Tvrdy <lukast.dev@gmail.com>
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

#ifndef _METABALL_H_
#define _METABALL_H_

#include <cmath>
#include <QtGlobal>

class Metaball
{
public:
    ~Metaball() {}
    Metaball(qreal x, qreal y, qreal radius):
            m_x(x),
            m_y(y),
            m_radius(radius) {}

    qreal equation(qreal x, qreal y) {
        //return m_radius / sqrt( pow((x - m_x),2) + pow((y - m_y),2) );
        return (m_radius*m_radius) / (pow((x - m_x), 2) + pow((y - m_y), 2));
    }

    qreal x() {
        return m_x;
    }

    qreal y() {
        return m_y;
    }

    qreal radius() {
        return m_radius;
    }
private:
    qreal m_x;
    qreal m_y;
    qreal m_radius;

};

#endif // _METABALL_H_

