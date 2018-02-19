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

#ifndef KOSHAPEHANDLESCOLLECTION_H
#define KOSHAPEHANDLESCOLLECTION_H

#include "kritaflake_export.h"

#include <KoHandleUtilityTypes.h>

class KoViewConverter;


class KRITAFLAKE_EXPORT KoShapeHandlesCollection
{
public:
    using HandlesVector = KoFlake::HandlesVector;
    using HandlesRecord = KoFlake::HandlesRecord;

    void addHandles(KoShape *shape, const KisHandleStyle &style, const HandlesVector &handles);
    void addHandles(KoShape *shape, const KisHandleStyle &style, const KoFlake::Handle &handle);

    void addHandles(const HandlesRecord &record);
    void addHandles(const QVector<HandlesRecord> &records);

    void drawHandles(QPainter *painter, const KoViewConverter &converter, qreal handleRadius);

    QRectF boundingRectDoc(const KoViewConverter &converter, qreal handleRadius);
    QVector<QRectF> updateDocRects(const KoViewConverter &converter, qreal handleRadius);
    QVector<QRectF> updateDocRects(qreal handleRadius);

    static QVector<QRectF> updateDocRects(const HandlesRecord &record, qreal handleRadius);
    static QVector<QRectF> updateDocRects(const QVector<HandlesRecord> &records, qreal handleRadius);

private:
    template <class Functor>
    void applyToAllHandles(QPainter *painter, const KoViewConverter &converter, qreal handleRadius, Functor &func);

private:
    QVector<HandlesRecord> m_handles;
};

#endif // KOSHAPEHANDLESCOLLECTION_H
