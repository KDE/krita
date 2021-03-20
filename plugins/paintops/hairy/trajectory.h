/*
 *  SPDX-FileCopyrightText: 2008-2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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

    inline int size() const {
        return m_size;
    }

private:
    QVector<QPointF> m_path;
    int m_i;
    int m_size;

private:
    void addPoint(QPointF pos);
    void reset();

};
#endif

