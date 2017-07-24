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

#ifndef KISSNAPPOINTSTRATEGY_H
#define KISSNAPPOINTSTRATEGY_H

#include <QScopedPointer>

#include "KoSnapStrategy.h"
#include "kritaui_export.h"

/**
 * The KisSnapPointStrategy class is a custom strategy that allows snapping to
 * arbitrary points on canvas, not linked to any real objects. It can be used,
 * for example, for snapping to the *previous position* of the handle, while it
 * is dragging by the user.
 */

class KRITAUI_EXPORT KisSnapPointStrategy : public KoSnapStrategy
{
public:
    KisSnapPointStrategy(KoSnapGuide::Strategy type = KoSnapGuide::CustomSnapping);
    ~KisSnapPointStrategy() override;

    bool snap(const QPointF &mousePosition, KoSnapProxy * proxy, qreal maxSnapDistance) override;
    QPainterPath decoration(const KoViewConverter &converter) const override;

     void addPoint(const QPointF &pt);

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif // KISSNAPPOINTSTRATEGY_H
