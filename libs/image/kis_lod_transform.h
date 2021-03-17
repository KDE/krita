/*
 *  SPDX-FileCopyrightText: 2014 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_LOD_TRANSFORM_H
#define __KIS_LOD_TRANSFORM_H

#include <kritaimage_export.h>
#include <brushengine/kis_paint_information.h>
#include "kis_lod_transform_base.h"

class KRITAIMAGE_EXPORT KisLodTransform : public KisLodTransformBase
{
public:

    KisLodTransform(int levelOfDetail)
        : KisLodTransformBase(levelOfDetail)
    {
    }

    template <class PaintDeviceTypeSP>
    KisLodTransform(PaintDeviceTypeSP device)
        : KisLodTransformBase(device->defaultBounds()->currentLevelOfDetail())
    {
    }

    using KisLodTransformBase::map;

    KisPaintInformation map(KisPaintInformation pi) const {
        QPointF pos = pi.pos();
        pi.setPos(this->map(pos));
        pi.setLevelOfDetail(m_levelOfDetail);
        return pi;
    }
};

#endif /* __KIS_LOD_TRANSFORM_H */
