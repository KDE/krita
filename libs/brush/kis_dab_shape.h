/*
 *  SPDX-FileCopyrightText: 2016 Nishant Rodrigues <nishantjr@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
