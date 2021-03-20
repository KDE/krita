/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2007, 2009 Jan Hambrecht <jaham@gmx.net>
 * SPDX-FileCopyrightText: 2010 Thorsten Zachmann <zachmann@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef SVGGRADIENTHELPER_H
#define SVGGRADIENTHELPER_H

#include <KoFlakeCoordinateSystem.h>
#include <QTransform>
#include <QGradient>
#include <SvgMeshGradient.h>

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

    /// Sets the meshgradient
    void setMeshGradient(SvgMeshGradient* g);
    /// Returns the meshgradient
    QScopedPointer<SvgMeshGradient>& meshgradient();

    // To distinguish between SvgMeshGradient and QGradient
    bool isMeshGradient() const;

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

    QScopedPointer<QGradient> m_gradient;
    QScopedPointer<SvgMeshGradient> m_meshgradient;
    KoFlake::CoordinateSystem m_gradientUnits;
    QTransform m_gradientTransform;
};

#endif // SVGGRADIENTHELPER_H
