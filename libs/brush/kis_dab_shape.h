/*
 *  Copyright (c) 2016 Nishant Rodrigues <nishantjr@gmail.com>
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

#pragma once
#include <QtGlobal>

class KisDabShape {
    qreal m_scale;
    qreal m_ratio;
    qreal m_rotation;

public:

    KisDabShape()
        : m_scale(1.0)
        , m_ratio(1.0)
        , m_rotation(0.0)
    {}
    KisDabShape(qreal scale, qreal ratio, qreal rotation)
        : m_scale(scale)
        , m_ratio(ratio)
        , m_rotation(rotation)
    {}

    bool operator==(const KisDabShape &rhs) const {
        return
            qFuzzyCompare(m_scale, rhs.m_scale) &&
            qFuzzyCompare(m_ratio, rhs.m_ratio) &&
            qFuzzyCompare(m_rotation, rhs.m_rotation);
    }

    qreal scale()    const { return m_scale; }
    qreal scaleX()   const { return scale(); }
    qreal scaleY()   const { return m_scale * m_ratio; }
    qreal ratio()    const { return m_ratio; }
    qreal rotation() const { return m_rotation; }
};
