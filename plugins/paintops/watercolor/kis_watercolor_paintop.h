/* This file is part of the KDE project
 *
 * Copyright (C) 2017 Grigory Tantsevov <tantsevov@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef KIS_EXPERIMENT_PAINTOP_H_
#define KIS_EXPERIMENT_PAINTOP_H_

#include <klocalizedstring.h>
#include <brushengine/kis_paintop.h>
#include <kis_types.h>
#include "kis_paint_information.h"

#include "kis_watercolorop_option.h"

#include <QElapsedTimer>

#include "kis_wetmap.h"
#include "kis_splat.h"
#include "KoRTree.h"

#include "kis_base_splats_plane.h"
#include "kis_fixed_splats_plane.h"

class KisPainter;

class KisWatercolorPaintOp : public KisPaintOp
{

public:

    KisWatercolorPaintOp(const KisPaintOpSettingsSP settings, KisPainter *painter, KisNodeSP node, KisImageSP image);
    ~KisWatercolorPaintOp();

    KisSpacingInformation paintAt(const KisPaintInformation& info) override;

    void updateSystem() override;

protected:

    KisSpacingInformation updateSpacingImpl(const KisPaintInformation &info) const override;

private:
    WatercolorOption m_watercolorOption;
    QElapsedTimer m_timer;
    qint16 m_lastTime;


    KisWetMap *m_wetMap;
//    QList<KisSplat *> m_flowing,
//                      m_fixed,
//                      m_dried;
//    KoRTree<KisSplat *> m_fixedTree;

    KisBaseSplatsPlane m_driedPlane;
    KisFixedSplatsPlane m_fixedPlane;
    KisBaseSplatsPlane m_flowingPlane;

    KisPaintDeviceSP m_oldPD;
};

#endif // KIS_EXPERIMENT_PAINTOP_H_
