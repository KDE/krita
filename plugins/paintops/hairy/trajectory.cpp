/*
 *  SPDX-FileCopyrightText: 2008-2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "trajectory.h"
#include <cmath>

#include <kis_debug.h>

Trajectory::Trajectory()
{
    m_i = 0;
    m_size = 0;
}

Trajectory::~Trajectory()
{
}


void Trajectory::addPoint(QPointF pos)
{
    if (m_i >= m_path.size()) {
        m_path.append(pos);
        m_i++;
    } else {
        m_path[m_i] = pos;
        m_i++;
    }

    m_size++;
}


void Trajectory::reset()
{
    m_size = 0;
    m_i = 0;
}


const QVector<QPointF> &Trajectory::getLinearTrajectory(const QPointF &start, const QPointF &end, double space)
{
    Q_UNUSED(space);
    reset();

    // Width and height of the line
    qreal xd = (end.x() - start.x());
    qreal yd = (end.y() - start.y());

    int x = (int)start.x();
    int y = (int)start.y();
    qreal fx = start.x();
    qreal fy = start.y();
    qreal m = yd / xd;

    int y2 = (int)end.y();
    int x2 = (int)end.x();

    addPoint(start);

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
            fy = fy + incr;
            y += incr;
//            x = (int)(fx + 0.5f);
            addPoint(QPointF(fx, fy));
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
            fx = fx + incr;
            x += incr;
//            y = (int)(fy + 0.5f);
            addPoint(QPointF(fx, fy));
        }
    }

    addPoint(end);
    return m_path;
}

QVector<QPointF> Trajectory::getDDATrajectory(QPointF start, QPointF end, double space)
{
    Q_UNUSED(space);
    reset();
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
        }
        else {
            m = -1.0f / m;
            incr = -1;
        }
        while (y != y2) {
            fx = fx + m;
            y = y + incr;
            x = (int)(fx + 0.5f);
            addPoint(QPointF(x, y));
        }
    } else {
        int incr;
        if (xd > 0) {
            incr = 1;
        }
        else {
            incr = -1;
            m = -m;
        }
        while (x != x2) {
            fy = fy + m;
            x = x + incr;
            y = (int)(fy + 0.5f);
            addPoint(QPointF(x, y));
        }
    }

    return m_path;
}


