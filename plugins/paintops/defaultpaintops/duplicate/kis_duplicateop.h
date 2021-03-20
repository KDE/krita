/*
 *  SPDX-FileCopyrightText: 2002 Patrick Julien <freak@codepimps.org>
 *  SPDX-FileCopyrightText: 2004-2008 Boudewijn Rempt <boud@valdyas.org>
 *  SPDX-FileCopyrightText: 2004 Clarence Dang <dang@kde.org>
 *  SPDX-FileCopyrightText: 2004 Adrian Page <adrian@pagenet.plus.com>
 *  SPDX-FileCopyrightText: 2004 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_DUPLICATEOP_H_
#define KIS_DUPLICATEOP_H_

#include "kis_brush_based_paintop.h"

#include <klocalizedstring.h>

#include <kis_types.h>
#include <brushengine/kis_paintop_factory.h>
#include <brushengine/kis_paintop_settings.h>
#include <kis_pressure_size_option.h>
#include <kis_pressure_rotation_option.h>
#include <kis_pressure_opacity_option.h>

#include "kis_duplicateop_settings.h"

class KisPaintInformation;


class QPointF;
class KisPainter;


class KisDuplicateOp : public KisBrushBasedPaintOp
{

public:

    KisDuplicateOp(const KisPaintOpSettingsSP settings, KisPainter *painter, KisNodeSP node, KisImageSP image);
    ~KisDuplicateOp() override;

protected:
    KisSpacingInformation paintAt(const KisPaintInformation& info) override;

    KisSpacingInformation updateSpacingImpl(const KisPaintInformation &info) const override;

private:

    qreal minimizeEnergy(const qreal* m, qreal* sol, int w, int h);

private:

    KisImageSP m_image;
    KisNodeSP m_node;

    KisDuplicateOpSettingsSP m_settings;
    KisPaintDeviceSP m_srcdev;
    KisPaintDeviceSP m_target;
    QPointF m_duplicateStart {QPointF(0.0, 0.0)};
    bool m_duplicateStartIsSet {false};
    KisPressureSizeOption m_sizeOption;
    KisPressureOpacityOption m_opacityOption;
    KisPressureRotationOption m_rotationOption;
    bool m_healing {false};
    bool m_perspectiveCorrection {false};
    bool m_moveSourcePoint {false};
    bool m_cloneFromProjection {false};
};

#endif // KIS_DUPLICATEOP_H_
