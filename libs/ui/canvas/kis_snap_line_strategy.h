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

#ifndef __KIS_SNAP_LINE_STRATEGY_H
#define __KIS_SNAP_LINE_STRATEGY_H

#include <QScopedPointer>

#include "KoSnapStrategy.h"


class KisSnapLineStrategy : public KoSnapStrategy
{
public:
    KisSnapLineStrategy(KoSnapGuide::Strategy type = KoSnapGuide::CustomSnapping);
    ~KisSnapLineStrategy() override;

    bool snap(const QPointF &mousePosition, KoSnapProxy * proxy, qreal maxSnapDistance) override;
    QPainterPath decoration(const KoViewConverter &converter) const override;

    void addLine(Qt::Orientation orientation, qreal pos);

    void setHorizontalLines(const QList<qreal> &lines);
    void setVerticalLines(const QList<qreal> &lines);

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif /* __KIS_SNAP_LINE_STRATEGY_H */
