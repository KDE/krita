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

#include "trajectory.h"
#include <cmath>

#include <kis_debug.h>

Trajectory::Trajectory()
{

}

QVector<QPointF> &Trajectory::getLinearTrajectory(const QPointF &start, const QPointF &end, double space)
{
    Q_UNUSED(space);
    m_path.clear();

    // Width and height of the line
    float xd = (end.x() - start.x());
    float yd = (end.y() - start.y());

    int x = (int)start.x();
    int y = (int)start.y();
    float fx = start.x();
    float fy = start.y();
    float m = yd / xd;

    int y2 = (int)end.y();
    int x2 = (int)end.x();

    m_path.append(start);

    if (fabs(m) > 1) {
        // y - directional axis
        int incr;
        if (yd > 0) {
            m = 1.0f / m;
            incr = 1;
        } else {
            m = -1.0f / m;
            incr = -1;
        }
        while (y != y2) {
            fx = fx + m;
            y = y + incr;
            x = (int)(fx + 0.5f);
            m_path.append(QPointF(fx, y));
        }
    } else {
        // x - directional axis
        int incr;
        if (xd > 0) {
            incr = 1;
        } else {
            incr = -1;
            m = -m;
        }
        while (x != x2) {
            fy = fy + m;
            x = x + incr;
            y = (int)(fy + 0.5f);
            m_path.append(QPointF(x, fy));
        }
    }

    m_path.append(end);
    return m_path;

}

QVector<QPointF> Trajectory::getDDATrajectory(QPointF start, QPointF end, double space)
{
    Q_UNUSED(space);
    m_path.clear();
    // Width and height of the line
    int xd = (int)(end.x() - start.x());
    int yd = (int)(end.y() - start.y());

    int x = (int)start.x();
    int y = (int)start.y();
    float fx = start.x();
    float fy = start.y();
    float m = (float)yd / (float)xd;
    int y2 = (int)end.y();
    int x2 = (int)end.x();

    if (fabs(m) > 1) {
        int incr;
        if (yd > 0) {
            m = 1.0f / m;
            incr = 1;
        } else {
            m = -1.0f / m;
            incr = -1;
        }
        while (y != y2) {
            fx = fx + m;
            y = y + incr;
            x = (int)(fx + 0.5f);
            m_path.append(QPointF(x, y));
        }
    } else {
        int incr;
        if (xd > 0) {
            incr = 1;
        } else {
            incr = -1;
            m = -m;
        }
        while (x != x2) {
            fy = fy + m;
            x = x + incr;
            y = (int)(fy + 0.5f);
            m_path.append(QPointF(x, y));
        }
    }

    return m_path;
}


Trajectory::~Trajectory()
{
}
