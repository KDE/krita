/*
 *  SPDX-FileCopyrightText: 2020 Sharaf Zaman <sharafzaz121@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
