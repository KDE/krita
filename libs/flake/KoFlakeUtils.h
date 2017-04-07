/*
 *  Copyright (c) 2016 Dmitry Kazakov <dimula73@gmail.com>
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

