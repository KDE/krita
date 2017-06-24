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

#ifndef __KIS_RECORDING_ADAPTER_H
#define __KIS_RECORDING_ADAPTER_H

#include "kis_types.h"
#include "kis_resources_snapshot.h"

class KisRecordedPathPaintAction;
class KisPaintInformation;
class KisDistanceInitInfo;


class KisRecordingAdapter
{
public:
    KisRecordingAdapter();
    ~KisRecordingAdapter();

    void startStroke(KisImageWSP image, KisResourcesSnapshotSP resources,
                     const KisDistanceInitInfo &startDist);
    void endStroke();

    void addPoint(const KisPaintInformation &pi);
    void addLine(const KisPaintInformation &pi1,
                 const KisPaintInformation &pi2);

    void addCurve(const KisPaintInformation &pi1,
                  const QPointF &control1,
                  const QPointF &control2,
                  const KisPaintInformation &pi2);

private:
    KisImageWSP m_image;
    KisRecordedPathPaintAction *m_pathPaintAction;
};

#endif /* __KIS_RECORDING_ADAPTER_H */
