/*
 *  Copyright (c) 2009-2010 Lukáš Tvrdý <lukast.dev@gmail.com>
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

#ifndef _DYNA_FILTER_H_
#define _DYNA_FILTER_H_

#include <QtGlobal>

class DynaFilter
{
public:
    DynaFilter() { initFilterPosition(0.0,0.0);  }
    ~DynaFilter() {}

    void initFilterPosition(qreal x, qreal y);
    bool applyFilter(qreal cursorX, qreal cursorY);
    void setFixedAngles(qreal angleX, qreal angleY);
/* setters */
    void setMass(qreal mass){ m_mass = mass;}
    void setDrag(qreal drag){ m_drag = drag;}
    void setUseFixedAngle(bool useFixed){ m_fixedAngle = useFixed; }
/* getters */
    qreal velocity() const { return m_vel; }
    qreal velocityX() const { return m_velx; }
    qreal velocityY() const { return m_vely; }

    qreal x() const { return m_filterX; }
    qreal y() const { return m_filterY; }
    qreal prevX() const { return m_prevX; }
    qreal prevY() const { return m_prevY; }
    qreal angleX() const { return m_angleX; }
    qreal angleY() const { return m_angleY; }
    qreal acceleration() const { return m_acc; }

private:
    qreal m_filterX, m_filterY;
    qreal m_velx, m_vely, m_vel; // velocity
    qreal m_accx, m_accy, m_acc; // acceleration
    qreal m_angleX, m_angleY;
    qreal m_prevX, m_prevY; // previous filtered X,Y

    bool m_fixedAngle;
    qreal m_fixedAngleX;
    qreal m_fixedAngleY;

    qreal m_mass;
    qreal m_drag;

private:
    // linear interpolation between f0 and f1
    qreal flerp(qreal f0, qreal f1, qreal p) const {
        return ((f0 *(1.0 - p)) + (f1 * p));
    }


};

#endif

