/*
 *  SPDX-FileCopyrightText: 2017 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
class SvgMeshGradient;

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
    const SvgMeshGradient* meshgradient() const;

    KUndo2Command* setColor(const QColor &color);
    KUndo2Command* setLineWidth(const float &lineWidth);

    KUndo2Command* setGradient(const QGradient *gradient, const QTransform &transform);
    KUndo2Command* applyGradient(const QGradient *gradient);
    KUndo2Command* applyGradientStopsOnly(const QGradient *gradient);

    KUndo2Command* setMeshGradient(const SvgMeshGradient *gradient, const QTransform &transform);

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif // KOSHAPEFILLWRAPPER_H
