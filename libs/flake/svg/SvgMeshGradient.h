/*
 *  Copyright (c) 2020 Sharaf Zaman <sharafzaz121@gmail.com>
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
#ifndef KISMESHGRADIENT_H
#define KISMESHGRADIENT_H

#include <QGradient>

#include <KoFlakeCoordinateSystem.h>
#include "SvgMeshPatch.h"
#include "SvgMeshArray.h"

class KRITAFLAKE_EXPORT SvgMeshGradient
{
public:
    enum Shading {
        BILINEAR,
        BICUBIC,
    };

    SvgMeshGradient();
    SvgMeshGradient(const SvgMeshGradient& other);

    void setType(Shading type);
    SvgMeshGradient::Shading type() const;

    void setTransform(const QTransform& matrix);
    bool isValid() const;

    void setGradientUnits(KoFlake::CoordinateSystem units = KoFlake::UserSpaceOnUse) {
        m_gradientUnits = units;
    }

    KoFlake::CoordinateSystem gradientUnits() const {
        return m_gradientUnits;
    }

    // returns boundingRect of the meshpatches in "user" coordinates (QPainter's)
    QRectF boundingRect() const;

    const QScopedPointer<SvgMeshArray>& getMeshArray() const;

private:
    Shading m_type;
    KoFlake::CoordinateSystem m_gradientUnits;
    QScopedPointer<SvgMeshArray> m_mesharray;
};

#endif // KISMESHGRADIENT_H
