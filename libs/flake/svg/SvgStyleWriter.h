/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2002 Lars Siebold <khandha5@gmx.net>
   SPDX-FileCopyrightText: 2002 Werner Trobin <trobin@kde.org>
   SPDX-FileCopyrightText: 2002 Lennart Kudling <kudling@kde.org>
   SPDX-FileCopyrightText: 2002-2003, 2005 Rob Buis <buis@kde.org>
   SPDX-FileCopyrightText: 2005 Boudewijn Rempt <boud@valdyas.org>
   SPDX-FileCopyrightText: 2005 Raphael Langerhorst <raphael.langerhorst@kdemail.net>
   SPDX-FileCopyrightText: 2005 Thomas Zander <zander@kde.org>
   SPDX-FileCopyrightText: 2005, 2008 Jan Hambrecht <jaham@gmx.net>
   SPDX-FileCopyrightText: 2006 Inge Wallin <inge@lysator.liu.se>
   SPDX-FileCopyrightText: 2006 Laurent Montel <montel@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef SVGSTYLEWRITER_H
#define SVGSTYLEWRITER_H

#include "kritaflake_export.h"
#include <QGradientStops>
#include <QSharedPointer>

class SvgSavingContext;
class KoShape;
class KoPatternBackground;
class KoVectorPatternBackground;
class QTransform;
class QGradient;
class SvgMeshGradient;

/// Helper class to save svg styles
class KRITAFLAKE_EXPORT SvgStyleWriter
{
public:
    /// Saves the style of the specified shape
    static void saveSvgStyle(KoShape *shape, SvgSavingContext &context);

    /// Saves only stroke, fill and transparency of the shape
    static void saveSvgBasicStyle(KoShape *shape, SvgSavingContext &context);

    /// Saves fill style of specified shape
    static void saveSvgFill(KoShape *shape, SvgSavingContext &context);
    /// Saves stroke style of specified shape
    static void saveSvgStroke(KoShape *shape, SvgSavingContext &context);

protected:
    /// Saves effects of specified shape
    static void saveSvgEffects(KoShape *shape, SvgSavingContext &context);
    /// Saves clipping of specified shape
    static void saveSvgClipping(KoShape *shape, SvgSavingContext &context);
    /// Saves masking of specified shape
    static void saveSvgMasking(KoShape *shape, SvgSavingContext &context);
    /// Saves markers of the path shape if present
    static void saveSvgMarkers(KoShape *shape, SvgSavingContext &context);
    /// Saves gradient color stops
    static void saveSvgColorStops(const QGradientStops &colorStops, SvgSavingContext &context);
    /// Saves gradient
    static QString saveSvgGradient(const QGradient *gradient, const QTransform &gradientTransform, SvgSavingContext &context);
    static QString saveSvgMeshGradient(SvgMeshGradient* gradient, const QTransform &transform, SvgSavingContext &context);
    /// Saves pattern
    static QString saveSvgPattern(QSharedPointer<KoPatternBackground> pattern, KoShape *shape, SvgSavingContext &context);
    static QString saveSvgVectorPattern(QSharedPointer<KoVectorPatternBackground> pattern, KoShape *shape, SvgSavingContext &context);
};

#endif // SVGSTYLEWRITER_H
