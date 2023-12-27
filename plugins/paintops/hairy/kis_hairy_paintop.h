/*
 *  SPDX-FileCopyrightText: 2008 Boudewijn Rempt <boud@valdyas.org>
 *  SPDX-FileCopyrightText: 2008-2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_HAIRYPAINTOP_H_
#define KIS_HAIRYPAINTOP_H_

#include <klocalizedstring.h>
#include <brushengine/kis_paintop.h>
#include <brushengine/kis_paintop_factory.h>
#include <kis_types.h>

#include "hairy_brush.h"

#include <KisStandardOptions.h>
#include <KisRotationOption.h>
#include <KisOpacityOption.h>
#include "KisHairyBristleOptionData.h"
#include "KisHairyInkOptionData.h"

class KisPainter;
class KisBrushBasedPaintOpSettings;
class KisResourcesInterface;

class KisHairyPaintOp : public KisPaintOp
{

public:
    KisHairyPaintOp(const KisPaintOpSettingsSP settings, KisPainter *painter, KisNodeSP node, KisImageSP image);

    void paintLine(const KisPaintInformation &pi1, const KisPaintInformation &pi2, KisDistanceInformation *currentDistance) override;

    static QList<KoResourceLoadResult> prepareLinkedResources(const KisPaintOpSettingsSP settings, KisResourcesInterfaceSP resourcesInterface);
protected:
    KisSpacingInformation paintAt(const KisPaintInformation& info) override;

    KisSpacingInformation updateSpacingImpl(const KisPaintInformation &info) const override;

private:
    KisHairyProperties m_properties;
    KisHairyBristleOptionData m_hairyBristleOption;
    KisHairyInkOptionData m_hairyInkOption;

    KisPaintDeviceSP m_dab;
    KisPaintDeviceSP m_dev;
    HairyBrush m_brush;
    KisOpacityOption m_opacityOption;
    KisSizeOption m_sizeOption;
    KisRotationOption m_rotationOption;

    void loadSettings();
};

#endif // KIS_HAIRYPAINTOP_H_
