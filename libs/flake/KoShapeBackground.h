/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2008 Jan Hambrecht <jaham@gmx.net>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KOSHAPEBACKGROUND_H
#define KOSHAPEBACKGROUND_H

#include "kritaflake_export.h"

#include <QtGlobal>

class QSizeF;
class QPainter;
class QPainterPath;
class KoShapeSavingContext;
class KoShapePaintingContext;

/**
 * This is the base class for shape backgrounds.
 * Derived classes are used to paint the background of
 * a shape within a given painter path.
 */
class KRITAFLAKE_EXPORT KoShapeBackground
{
public:
    KoShapeBackground();
    virtual ~KoShapeBackground();

    /// Paints the background using the given fill path
    virtual void paint(QPainter &painter, KoShapePaintingContext &context, const QPainterPath &fillPath) const = 0;

    /// Returns if the background has some transparency.
    virtual bool hasTransparency() const;

    virtual bool compareTo(const KoShapeBackground *other) const = 0;

    virtual explicit operator bool() const { return true; }

};

#endif // KOSHAPEBACKGROUND_H
