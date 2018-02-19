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
#include <KoViewConverter.h>
#include "KisHandlePainterHelper.h"


void KoShapeHandlesCollection::addHandles(KoShape *shape,
                                          const KisHandleStyle &style,
                                          const KoFlake::HandlesVector &handles)
{
    m_handles << HandlesRecord(shape, style, handles);
}

void KoShapeHandlesCollection::addHandles(KoShape *shape, const KisHandleStyle &style, const KoFlake::Handle &handle)
{
    addHandles(shape, style, HandlesVector({handle}));
}

void KoShapeHandlesCollection::addHandles(const KoFlake::HandlesRecord &record)
{
    m_handles << record;
}

void KoShapeHandlesCollection::addHandles(const QVector<KoFlake::HandlesRecord> &records)
{
    m_handles << records;
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
        KoShape *shape = it->shape;

        if (shape != lastPaintedShape) {
            lastPaintedShape = shape;
            helper = std::move(KoShape::createHandlePainterHelper(painter, shape, converter, handleRadius));
        }

        helper.setHandleStyle(it->style);

        Q_FOREACH (const KritaUtils::Handle &handle, it->handles) {
            func(helper, handle);
        }
    }
}

void KoShapeHandlesCollection::drawHandles(QPainter *painter, const KoViewConverter &converter, qreal handleRadius)
{
    auto drawHandleFunc =
            [] (KisHandlePainterHelper &helper, const KritaUtils::Handle &handle) {
        helper.drawHandle(handle);
    };

    applyToAllHandles(painter, converter, handleRadius, drawHandleFunc);
}

QRectF KoShapeHandlesCollection::boundingRectDoc(const KoViewConverter &converter, qreal handleRadius)
{
    QRectF boundingRect;

    auto drawHandleFunc =
            [&boundingRect] (KisHandlePainterHelper &helper, const KritaUtils::Handle &handle) {
        boundingRect |= helper.handleBoundingRectDoc(handle);
    };

    applyToAllHandles(0, converter, handleRadius, drawHandleFunc);

    return boundingRect;
}

QVector<QRectF> KoShapeHandlesCollection::updateDocRects(const KoViewConverter &converter, qreal handleRadius)
{
    QVector<QRectF> result;

    auto drawHandleFunc =
        [&result] (KisHandlePainterHelper &helper, const KritaUtils::Handle &handle) {
            result << helper.handleBoundingRectDoc(handle);
        };

    applyToAllHandles(0, converter, handleRadius, drawHandleFunc);

    return result;
}

QVector<QRectF> KoShapeHandlesCollection::updateDocRects(qreal handleRadius)
{
    KoViewConverter fakeConverter;
    return updateDocRects(fakeConverter, handleRadius);
}

QVector<QRectF> KoShapeHandlesCollection::updateDocRects(const KoFlake::HandlesRecord &record, qreal handleRadius)
{
    return updateDocRects(QVector<HandlesRecord>({record}), handleRadius);
}

QVector<QRectF> KoShapeHandlesCollection::updateDocRects(const QVector<KoFlake::HandlesRecord> &records, qreal handleRadius)
{
    KoShapeHandlesCollection collection;
    collection.addHandles(records);
    return collection.updateDocRects(handleRadius);
}
