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

#ifndef __KIS_COLORIZE_STROKE_STRATEGY_H
#define __KIS_COLORIZE_STROKE_STRATEGY_H

#include <QScopedPointer>
#include <QObject>

#include "kis_types.h"
#include <kis_simple_stroke_strategy.h>

class KoColor;


class KisColorizeStrokeStrategy : public QObject, public KisSimpleStrokeStrategy
{
    Q_OBJECT

public:
    KisColorizeStrokeStrategy(KisPaintDeviceSP src,
                              KisPaintDeviceSP dst,
                              KisPaintDeviceSP filteredSource,
                              bool filteredSourceValid,
                              const QRect &boundingRect, KisNodeSP dirtyNode);
    KisColorizeStrokeStrategy(const KisColorizeStrokeStrategy &rhs, int levelOfDetail);
    ~KisColorizeStrokeStrategy() override;

    void addKeyStroke(KisPaintDeviceSP dev, const KoColor &color);

    void initStrokeCallback() override;

    KisStrokeStrategy *createLodClone(int levelOfDetail) override;

Q_SIGNALS:
    void sigFinished();

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif /* __KIS_COLORIZE_STROKE_STRATEGY_H */
