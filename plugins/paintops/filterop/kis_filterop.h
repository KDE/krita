/*
 *  SPDX-FileCopyrightText: 2002 Patrick Julien <freak@codepimps.org>
 *  SPDX-FileCopyrightText: 2004-2008 Boudewijn Rempt <boud@valdyas.org>
 *  SPDX-FileCopyrightText: 2004 Clarence Dang <dang@kde.org>
 *  SPDX-FileCopyrightText: 2004 Adrian Page <adrian@pagenet.plus.com>
 *  SPDX-FileCopyrightText: 2004 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_FILTEROP_H_
#define KIS_FILTEROP_H_

#include "kis_brush_based_paintop.h"
#include <kis_pressure_size_option.h>
#include <kis_pressure_rotation_option.h>

class KisFilterConfiguration;
class KisFilterOpSettings;
class KisPaintInformation;
class KisPainter;

class KisFilterOp : public KisBrushBasedPaintOp
{

public:

    KisFilterOp(const KisPaintOpSettingsSP settings, KisPainter * painter, KisNodeSP node, KisImageSP image);
    ~KisFilterOp() override;

    static QList<KoResourceLoadResult> prepareLinkedResources(const KisPaintOpSettingsSP settings, KisResourcesInterfaceSP resourcesInterface);
    static QList<KoResourceLoadResult> prepareEmbeddedResources(const KisPaintOpSettingsSP settings, KisResourcesInterfaceSP resourcesInterface);

protected:

    KisSpacingInformation paintAt(const KisPaintInformation& info) override;

    KisSpacingInformation updateSpacingImpl(const KisPaintInformation &info) const override;

private:

    KisPaintDeviceSP m_tmpDevice;
    KisPressureSizeOption m_sizeOption;
    KisPressureRotationOption m_rotationOption;
    KisFilterSP m_filter;
    KisFilterConfigurationSP m_filterConfiguration;
    bool m_smudgeMode;
};

#endif // KIS_FILTEROP_H_
