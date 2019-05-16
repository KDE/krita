/*
 *  Copyright (c) 2017 Dmitry Kazakov <dimula73@gmail.com>
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

#ifndef KOSHAPEFILLWRAPPER_H
#define KOSHAPEFILLWRAPPER_H

#include "kritaflake_export.h"
#include <QScopedPointer>
#include <QList>
#include <KoFlake.h>

class KUndo2Command;
class KoShape;
class QColor;
class QTransform;
class QGradient;

class KRITAFLAKE_EXPORT KoShapeFillWrapper
{
public:
    KoShapeFillWrapper(KoShape *shape, KoFlake::FillVariant fillVariant);
    KoShapeFillWrapper(QList<KoShape*> shapes, KoFlake::FillVariant fillVariant);

    ~KoShapeFillWrapper();

    bool isMixedFill() const;
    KoFlake::FillType type() const;

    QColor color() const;
    const QGradient* gradient() const;
    QTransform gradientTransform() const;
    bool hasZeroLineWidth() const;

    KUndo2Command* setColor(const QColor &color);
    KUndo2Command* setLineWidth(const float &lineWidth);

    KUndo2Command* setGradient(const QGradient *gradient, const QTransform &transform);
    KUndo2Command* applyGradient(const QGradient *gradient);
    KUndo2Command* applyGradientStopsOnly(const QGradient *gradient);

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif // KOSHAPEFILLWRAPPER_H
