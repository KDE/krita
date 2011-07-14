/*
 *  Copyright (c) 2008-2010 Lukáš Tvrdý <lukast.dev@gmail.com>
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

#ifndef _TRAJECTORY_H_
#define _TRAJECTORY_H_


#include <QVector>
#include <QPointF>

class Trajectory
{

public:
    Trajectory();
    ~Trajectory();
    const QVector<QPointF> &getLinearTrajectory(const QPointF &start, const QPointF &end, double space);
    QVector<QPointF> getDDATrajectory(QPointF start, QPointF end, double space);

    inline int size() const { return m_size; }

private:
    QVector<QPointF> m_path;
    int m_i;
    int m_size;

private:
    void addPoint(QPointF pos);
    void reset();

};
#endif

