/* This file is part of the KDE project
 * Copyright (C) 2007,2009 Jan Hambrecht <jaham@gmx.net>
 * Copyright (C) 2010 Thorsten Zachmann <zachmann@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef SVGGRADIENTHELPER_H
#define SVGGRADIENTHELPER_H

#include <KoFlakeCoordinateSystem.h>
#include <QTransform>
#include <QGradient>

class SvgGradientHelper
{
public:
    SvgGradientHelper();
    ~SvgGradientHelper();
    /// Copy constructor
    SvgGradientHelper(const SvgGradientHelper &other);

    /// Sets the gradient units type
    void setGradientUnits(KoFlake::CoordinateSystem units);
    /// Returns gradient units type
    KoFlake::CoordinateSystem gradientUnits() const;

    /// Sets the gradient
    void setGradient(QGradient * g);
    /// Retrurns the gradient
    QGradient * gradient() const;

    /// Returns the gradient transformation
    QTransform transform() const;
    /// Sets the gradient transformation
    void setTransform(const QTransform &transform);

    /// Assignment operator
    SvgGradientHelper & operator = (const SvgGradientHelper & rhs);

    QGradient * adjustedGradient(const QRectF &bound) const;

    /// Converts a gradient from LogicalMode to ObjectBoundingMode 
    static QGradient *convertGradient(const QGradient * originalGradient, const QTransform &userToRelativeTransform, const QRectF &size);

    QGradient::Spread spreadMode() const;
    void setSpreadMode(const QGradient::Spread &spreadMode);

private:

    QGradient * m_gradient;
    KoFlake::CoordinateSystem m_gradientUnits;
    QTransform m_gradientTransform;
};

#endif // SVGGRADIENTHELPER_H
