/* This file is part of the KDE project
   Copyright (C) 2002 Lars Siebold <khandha5@gmx.net>
   Copyright (C) 2002 Werner Trobin <trobin@kde.org>
   Copyright (C) 2002 Lennart Kudling <kudling@kde.org>
   Copyright (C) 2002-2003,2005 Rob Buis <buis@kde.org>
   Copyright (C) 2005 Boudewijn Rempt <boud@valdyas.org>
   Copyright (C) 2005 Raphael Langerhorst <raphael.langerhorst@kdemail.net>
   Copyright (C) 2005 Thomas Zander <zander@kde.org>
   Copyright (C) 2005,2008 Jan Hambrecht <jaham@gmx.net>
   Copyright (C) 2006 Inge Wallin <inge@lysator.liu.se>
   Copyright (C) 2006 Laurent Montel <montel@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#ifndef SVGSTYLEWRITER_H
#define SVGSTYLEWRITER_H

#include "flake_export.h"
#include <QGradientStops>
#include <QSharedPointer>

class SvgSavingContext;
class KoShape;
class KoPatternBackground;
class QTransform;
class QGradient;

/// Helper class to save svg styles
class FLAKE_EXPORT SvgStyleWriter
{
public:
    /// Saves the style of the specified shape
    static void saveSvgStyle(KoShape *shape, SvgSavingContext &context);

protected:
    /// Saves fill style of specified shape
    static void saveSvgFill(KoShape *shape, SvgSavingContext &context);
    /// Saves stroke style of specified shape
    static void saveSvgStroke(KoShape *shape, SvgSavingContext &context);
    /// Saves effects of specified shape
    static void saveSvgEffects(KoShape *shape, SvgSavingContext &context);
    /// Saves clipping of specified shape
    static void saveSvgClipping(KoShape *shape, SvgSavingContext &context);
    /// Saves gradient color stops
    static void saveSvgColorStops(const QGradientStops &colorStops, SvgSavingContext &context);
    /// Saves gradient
    static QString saveSvgGradient(const QGradient *gradient, const QTransform &gradientTransform, SvgSavingContext &context);
    /// Saves pattern
    static QString saveSvgPattern(QSharedPointer<KoPatternBackground> pattern, KoShape *shape, SvgSavingContext &context);
};

#endif // SVGSTYLEWRITER_H
