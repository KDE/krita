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

#include <QtGui/QGradient>
#include <QtGui/QConicalGradient>
#include <QtGui/QLinearGradient>
#include <QtGui/QRadialGradient>

#include <kdebug.h>

class SvgGradientHelper
{
public:
    enum Units { UserSpaceOnUse, ObjectBoundingBox };

    SvgGradientHelper();
    ~SvgGradientHelper();
    /// Copy constructor
    SvgGradientHelper(const SvgGradientHelper &other);

    /// Sets the gradient units type
    void setGradientUnits(Units units);
    /// Returns gradient units type
    Units gradientUnits() const;

    /// Sets the gradient
    void setGradient(QGradient * g);
    /// Retrurns the gradient
    QGradient * gradient();

    /// Copies the given gradient
    void copyGradient(QGradient * g);

    /// Returns fill adjusted to the given bounding box
    QBrush adjustedFill(const QRectF &bound);

    /// Returns the gradient transformation
    QTransform transform() const;
    /// Sets the gradient transformation
    void setTransform(const QTransform &transform);

    /// Assigment operator
    SvgGradientHelper & operator = (const SvgGradientHelper & rhs);

    QGradient * adjustedGradient(const QRectF &bound) const;

    /// Converts a gradient from LogicalMode to ObjectBoundingMode 
    static QGradient *convertGradient(const QGradient * originalGradient, const QSizeF &size);

private:

    /// Duplicates the given gradient and applies the given transformation
    static QGradient *duplicateGradient(const QGradient * g, const QTransform &transform);

    QGradient * m_gradient;
    Units m_gradientUnits;
    QTransform m_gradientTransform;
};

#endif // SVGGRADIENTHELPER_H
