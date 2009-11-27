/*
 *  Copyright (c) 2009 Lukáš Tvrdý <lukast.dev@gmail.com>
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

#include "filter.h"
#include <cmath>

#define MIN_MASS 5.0
#define MAX_MASS 160.0
#define MIN_DRAG 0.0
#define MAX_DRAG 1.0
#define MIN_ACC 0.000001
#define MIN_VEL 0.000001

bool DynaFilter::applyFilter(qreal cursorX, qreal cursorY)
{
    qreal mass, drag;
    qreal fx, fy;

    /* calculate mass and drag */
    mass = flerp(MIN_MASS, MAX_MASS, m_mass);
    drag = flerp(MIN_DRAG, MAX_DRAG, m_drag * m_drag);

    /* calculate force and acceleration */
    fx = cursorX - m_filterX;
    fy = cursorY - m_filterY;

    m_acc = sqrt(fx * fx + fy * fy);

    if (m_acc < MIN_ACC) {
        return false;
    }

    // f = m*a (force = mass * acceleration)
    // a = f/m (so acceleration = force/mass)
    m_accx = fx / mass;
    m_accy = fy / mass;

    /* calculate new velocity */
    m_velx += m_accx;
    m_vely += m_accy;
    m_vel = sqrt(m_velx * m_velx + m_vely * m_vely);
    m_angleX = -m_vely;
    m_angleY = m_velx;

    if (m_vel < MIN_VEL) {
        return false;
    }

    /* calculate angle of drawing tool */
    m_angleX /= m_vel;
    m_angleY /= m_vel;
    if (m_fixedAngle) {
        m_angleX = m_fixedAngleX;
        m_angleY = m_fixedAngleY;
    }

    m_velx = m_velx * (1.0 - drag);
    m_vely = m_vely * (1.0 - drag);

    m_prevX = m_filterX;
    m_prevY = m_filterY;
    m_filterX = m_filterX + m_velx;
    m_filterY = m_filterY + m_vely;

    return true;
}


void DynaFilter::initialize(qreal x, qreal y)
{
    m_filterX = x;
    m_filterY = y;
    m_prevX = x;
    m_prevY = y;
    m_velx = 0.0;
    m_vely = 0.0;
    m_accx = 0.0;
    m_accy = 0.0;
}

void DynaFilter::setFixedAngles(qreal angleX, qreal angleY)
{
    m_fixedAngleX = angleX;
    m_fixedAngleY = angleY;
}

