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

#ifndef __FREEHAND_STROKE_H
#define __FREEHAND_STROKE_H

#include "kritaui_export.h"
#include "kis_types.h"
#include "kis_node.h"
#include "kis_painter_based_stroke_strategy.h"
#include <kis_distance_information.h>
#include <brushengine/kis_paint_information.h>
#include "kis_lod_transform.h"
#include "KoColor.h"



class KRITAUI_EXPORT FreehandStrokeStrategy : public KisPainterBasedStrokeStrategy
{
public:
    class Data : public KisStrokeJobData {
    public:
        enum DabType {
            POINT,
            LINE,
            CURVE,
            POLYLINE,
            POLYGON,
            RECT,
            ELLIPSE,
            PAINTER_PATH,
            QPAINTER_PATH,
            QPAINTER_PATH_FILL
        };

        Data(int _strokeInfoId,
             const KisPaintInformation &_pi)
            : KisStrokeJobData(KisStrokeJobData::UNIQUELY_CONCURRENT),
              strokeInfoId(_strokeInfoId),
              type(POINT), pi1(_pi)
        {}

        Data(int _strokeInfoId,
             const KisPaintInformation &_pi1,
             const KisPaintInformation &_pi2)
            : KisStrokeJobData(KisStrokeJobData::UNIQUELY_CONCURRENT),
              strokeInfoId(_strokeInfoId),
              type(LINE), pi1(_pi1), pi2(_pi2)
        {}

        Data(int _strokeInfoId,
             const KisPaintInformation &_pi1,
             const QPointF &_control1,
             const QPointF &_control2,
             const KisPaintInformation &_pi2)
            : KisStrokeJobData(KisStrokeJobData::UNIQUELY_CONCURRENT),
              strokeInfoId(_strokeInfoId),
              type(CURVE), pi1(_pi1), pi2(_pi2),
              control1(_control1), control2(_control2)
        {}

        Data(int _strokeInfoId,
             DabType _type,
             const vQPointF &_points)
            : KisStrokeJobData(KisStrokeJobData::UNIQUELY_CONCURRENT),
              strokeInfoId(_strokeInfoId),
            type(_type), points(_points)
        {}

        Data(int _strokeInfoId,
             DabType _type,
             const QRectF &_rect)
            : KisStrokeJobData(KisStrokeJobData::UNIQUELY_CONCURRENT),
              strokeInfoId(_strokeInfoId),
            type(_type), rect(_rect)
        {}

        Data(int _strokeInfoId,
             DabType _type,
             const QPainterPath &_path)
            : KisStrokeJobData(KisStrokeJobData::UNIQUELY_CONCURRENT),
              strokeInfoId(_strokeInfoId),
            type(_type), path(_path)
        {}

        Data(int _strokeInfoId,
             DabType _type,
             const QPainterPath &_path,
             const QPen &_pen, const KoColor &_customColor)
            : KisStrokeJobData(KisStrokeJobData::UNIQUELY_CONCURRENT),
              strokeInfoId(_strokeInfoId),
            type(_type), path(_path),
            pen(_pen), customColor(_customColor)
        {}

        KisStrokeJobData* createLodClone(int levelOfDetail) override {
            return new Data(*this, levelOfDetail);
        }

    private:
        Data(const Data &rhs, int levelOfDetail)
            : KisStrokeJobData(rhs),
              strokeInfoId(rhs.strokeInfoId),
              type(rhs.type)
        {
            KisLodTransform t(levelOfDetail);

            switch(type) {
            case Data::POINT:
                pi1 = t.map(rhs.pi1);
                break;
            case Data::LINE:
                pi1 = t.map(rhs.pi1);
                pi2 = t.map(rhs.pi2);
                break;
            case Data::CURVE:
                pi1 = t.map(rhs.pi1);
                pi2 = t.map(rhs.pi2);
                control1 = t.map(rhs.control1);
                control2 = t.map(rhs.control2);
                break;
            case Data::POLYLINE:
                points = t.map(rhs.points);
                break;
            case Data::POLYGON:
                points = t.map(rhs.points);
                break;
            case Data::RECT:
                rect = t.map(rhs.rect);
                break;
            case Data::ELLIPSE:
                rect = t.map(rhs.rect);
                break;
            case Data::PAINTER_PATH:
                path = t.map(rhs.path);
                break;
            case Data::QPAINTER_PATH:
                path = t.map(rhs.path);
                pen = rhs.pen;
                break;
            case Data::QPAINTER_PATH_FILL:
                path = t.map(rhs.path);
                pen = rhs.pen;
                customColor = rhs.customColor;
                break;
            };
        }
    public:
        int strokeInfoId;

        DabType type;
        KisPaintInformation pi1;
        KisPaintInformation pi2;
        QPointF control1;
        QPointF control2;

        vQPointF points;
        QRectF rect;
        QPainterPath path;
        QPen pen;
        KoColor customColor;
    };

public:
    FreehandStrokeStrategy(KisResourcesSnapshotSP resources,
                           KisFreehandStrokeInfo *strokeInfo,
                           const KUndo2MagicString &name);

    FreehandStrokeStrategy(KisResourcesSnapshotSP resources,
                           QVector<KisFreehandStrokeInfo*> strokeInfos,
                           const KUndo2MagicString &name);

    ~FreehandStrokeStrategy() override;

    void initStrokeCallback() override;
    void finishStrokeCallback() override;

    void doStrokeCallback(KisStrokeJobData *data) override;

    KisStrokeStrategy* createLodClone(int levelOfDetail) override;

    void notifyUserStartedStroke() override;
    void notifyUserEndedStroke() override;

protected:
    FreehandStrokeStrategy(const FreehandStrokeStrategy &rhs, int levelOfDetail);

private:
    void init();

    void tryDoUpdate(bool forceEnd = false);
    void issueSetDirtySignals();

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif /* __FREEHAND_STROKE_H */
