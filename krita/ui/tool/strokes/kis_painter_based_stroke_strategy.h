/*
 *  Copyright (c) 2011 Dmitry Kazakov <dimula73@gmail.com>
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

#ifndef __KIS_PAINTER_BASED_STROKE_STRATEGY_H
#define __KIS_PAINTER_BASED_STROKE_STRATEGY_H

#include "kis_simple_stroke_strategy.h"
#include "kis_resources_snapshot.h"
#include "kis_selection.h"

class KisPainter;
struct KisDistanceInformation;
class KisTransaction;


class KRITAUI_EXPORT KisPainterBasedStrokeStrategy : public KisSimpleStrokeStrategy
{
public:
    /**
     * The distance information should be associated with each
     * painter individually, so we strore and manipulate with
     * them together using the structure PainterInfo
     */
    class KRITAUI_EXPORT PainterInfo {
    public:
        PainterInfo(KisPainter *painter, KisDistanceInformation *dragDistance);
        ~PainterInfo();

        KisPainter *painter;
        KisDistanceInformation *dragDistance;
    };

public:
    KisPainterBasedStrokeStrategy(const QString &id,
                                  const QString &name,
                                  KisResourcesSnapshotSP resources,
                                  QVector<PainterInfo*> painterInfos);

    KisPainterBasedStrokeStrategy(const QString &id,
                                  const QString &name,
                                  KisResourcesSnapshotSP resources,
                                  PainterInfo *painterInfo);

    void initStrokeCallback();
    void finishStrokeCallback();
    void cancelStrokeCallback();

protected:
    KisPaintDeviceSP targetDevice();
    KisSelectionSP activeSelection();

private:
    void init();
    void initPainters(KisPaintDeviceSP targetDevice,
                      KisSelectionSP selection,
                      bool hasIndirectPainting,
                      const QString &indirectPaintingCompositeOp);
    void deletePainters();

private:
    KisResourcesSnapshotSP m_resources;
    QVector<PainterInfo*> m_painterInfos;
    KisTransaction *m_transaction;

    KisPaintDeviceSP m_targetDevice;
    KisSelectionSP m_activeSelection;
};

#endif /* __KIS_PAINTER_BASED_STROKE_STRATEGY_H */
