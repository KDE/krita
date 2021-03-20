/*
 *  SPDX-FileCopyrightText: 2009 Lukas Tvrdy <lukast.dev@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
        return (m_radius * m_radius) / (pow((x - m_x), 2) + pow((y - m_y), 2));
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

