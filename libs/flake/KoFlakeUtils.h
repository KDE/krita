/*
 *  SPDX-FileCopyrightText: 2016 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KOFLAKEUTILS_H
#define KOFLAKEUTILS_H

#include <KoShape.h>
#include <KoFlakeTypes.h>
#include <KoShapeStroke.h>

#include "kis_global.h"
#include "KoShapeStrokeCommand.h"


namespace KoFlake {

template <typename ModifyFunction>
    auto modifyShapesStrokes(QList<KoShape*> shapes, ModifyFunction modifyFunction)
        -> decltype(modifyFunction(KoShapeStrokeSP()), (KUndo2Command*)(0))
    {
        if (shapes.isEmpty()) return 0;

        QList<KoShapeStrokeModelSP> newStrokes;

        Q_FOREACH(KoShape *shape, shapes) {
            KoShapeStrokeSP shapeStroke = shape->stroke() ?
                qSharedPointerDynamicCast<KoShapeStroke>(shape->stroke()) :
                KoShapeStrokeSP();

            KoShapeStrokeSP newStroke =
                toQShared(shapeStroke ?
                              new KoShapeStroke(*shapeStroke) :
                              new KoShapeStroke());

            modifyFunction(newStroke);

            newStrokes << newStroke;
        }

        return new KoShapeStrokeCommand(shapes, newStrokes);
}

template <class Policy>
bool compareShapePropertiesEqual(const QList<KoShape*> shapes, const Policy &policy)
{
    if (shapes.size() == 1) return true;

    typename Policy::PointerType bg =
            policy.getProperty(shapes.first());

    Q_FOREACH (KoShape *shape, shapes) {
        typename Policy::PointerType otherBg = policy.getProperty(shape);

        if (
            !(
                (!bg && !otherBg) ||
                (bg && otherBg && policy.compareTo(bg, otherBg))
                )) {

            return false;
        }
    }

    return true;
}

template <class Policy>
bool compareShapePropertiesEqual(const QList<KoShape*> shapes)
{
    return compareShapePropertiesEqual<Policy>(shapes, Policy());
}

}

#endif // KOFLAKEUTILS_H

