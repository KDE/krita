/*
 *  Copyright (c) 2007 Cyrille Berger <cberger@cberger.net>
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

#ifndef _KIS_RECORDED_PATH_PAINT_ACTIONS_H_
#define _KIS_RECORDED_PATH_PAINT_ACTIONS_H_

#include "recorder/kis_recorded_action.h"
#include "recorder/kis_recorded_paint_action.h"
#include "kis_types.h"

class KisPaintInformation;
class KisPainter;
class KisDistanceInitInfo;

#include <kritaimage_export.h>

/**
 * This class will record the painting of a bezier curve.
 */
class KRITAIMAGE_EXPORT KisRecordedPathPaintAction : public KisRecordedPaintAction
{

public:

    /**
     * @param startDist - Provides initial information related to distance and spacing, which can
     * have an effect on how the path is painted.
     */
    KisRecordedPathPaintAction(const KisNodeQueryPath& path,
                               const KisPaintOpPresetSP paintOpPreset,
                               const KisDistanceInitInfo& startDistInfo);

    KisRecordedPathPaintAction(const KisRecordedPathPaintAction&);

    ~KisRecordedPathPaintAction() override;

    KisDistanceInitInfo getInitDistInfo() const;

    void setInitDistInfo(const KisDistanceInitInfo &startDistInfo);

    void addPoint(const KisPaintInformation& info);
    void addLine(const KisPaintInformation& point1, const KisPaintInformation& point2);
    void addPolyLine(const QList<QPointF>& points);
    void addCurve(const KisPaintInformation& point1,
                  const QPointF& control1,
                  const QPointF& control2,
                  const KisPaintInformation& point2);

    void toXML(QDomDocument& doc, QDomElement& elt, KisRecordedActionSaveContext* ) const override;

    KisRecordedAction* clone() const override;

protected:

    void playPaint(const KisPlayInfo& info, KisPainter* painter) const override;

private:

    struct Private;
    Private* const d;
};


class KisRecordedPathPaintActionFactory : public KisRecordedPaintActionFactory
{
public:
    KisRecordedPathPaintActionFactory();
    ~KisRecordedPathPaintActionFactory() override;
    KisRecordedAction* fromXML(const QDomElement& elt, const KisRecordedActionLoadContext*) override;
};

#endif
