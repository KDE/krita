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

#ifndef __KIS_COLOR_PICKER_STROKE_STRATEGY_H
#define __KIS_COLOR_PICKER_STROKE_STRATEGY_H

#include <QObject>
#include "kis_simple_stroke_strategy.h"
#include "kis_lod_transform.h"
#include "KoColor.h"

class KisColorPickerStrokeStrategy : public QObject, public KisSimpleStrokeStrategy
{
    Q_OBJECT
public:
    class Data : public KisStrokeJobData {
    public:
        Data(KisPaintDeviceSP _dev, const QPoint _pt, KoColor _currentColor)
            : dev(_dev), pt(_pt), currentColor(_currentColor)
        {}

        KisStrokeJobData* createLodClone(int levelOfDetail) override {
            KisLodTransform t(levelOfDetail);
            const QPoint realPoint = t.map(pt);

            return new Data(dev, realPoint, currentColor);
        }

        KisPaintDeviceSP dev;
        QPoint pt;
        KoColor currentColor; // Used for color picker blending.
    };
public:
    KisColorPickerStrokeStrategy(int lod = 0);
    ~KisColorPickerStrokeStrategy() override;

    void doStrokeCallback(KisStrokeJobData *data) override;
    KisStrokeStrategy* createLodClone(int levelOfDetail) override;

Q_SIGNALS:
    void sigColorUpdated(const KoColor &color);

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif /* __KIS_COLOR_PICKER_STROKE_STRATEGY_H */
