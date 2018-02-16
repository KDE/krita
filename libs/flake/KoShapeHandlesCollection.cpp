/*
 *  Copyright (c) 2018 Dmitry Kazakov <dimula73@gmail.com>
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

#include "KoShapeHandlesCollection.h"

#include <tuple>
#include <KoShape.h>


void KoShapeHandlesCollection::addHandles(KoShape *shape,
                                          const KisHandleStyle &style,
                                          const KoShapeHandlesCollection::HandlesVector &handles)
{
    m_handles << std::make_tuple(shape, style, handles);
}

template <class Functor>
void KoShapeHandlesCollection::applyToAllHandles(QPainter *painter,
                                                 const KoViewConverter &converter,
                                                 qreal handleRadius,
                                                 Functor &func)
{
    KisHandlePainterHelper helper;
    KoShape *lastPaintedShape = 0;

    for (auto it = m_handles.begin(); it != m_handles.end(); ++it) {
        KoShape *shape = std::get<0>(*it);

        if (shape != lastPaintedShape) {
            lastPaintedShape = shape;
            helper = std::move(KoShape::createHandlePainterHelper(painter, shape, converter, handleRadius));
        }

        const KisHandleStyle &style = std::get<1>(*it);
        const HandlesVector &handles = std::get<2>(*it);

        helper.setHandleStyle(style);

        Q_FOREACH (const KisHandlePainterHelper::Handle &handle, handles) {
            func(helper, handle);
        }
    }
}

void KoShapeHandlesCollection::drawHandles(QPainter *painter, const KoViewConverter &converter, qreal handleRadius)
{
    auto drawHandleFunc =
            [] (KisHandlePainterHelper &helper, const KisHandlePainterHelper::Handle &handle) {
        helper.drawHandle(handle);
    };

    applyToAllHandles(painter, converter, handleRadius, drawHandleFunc);
}

QRectF KoShapeHandlesCollection::boundingRectDoc(const KoViewConverter &converter, qreal handleRadius)
{
    QRectF boundingRect;

    auto drawHandleFunc =
            [&boundingRect] (KisHandlePainterHelper &helper, const KisHandlePainterHelper::Handle &handle) {
        boundingRect |= helper.handleBoundingRectDoc(handle);
    };

    applyToAllHandles(0, converter, handleRadius, drawHandleFunc);

    return boundingRect;
}
