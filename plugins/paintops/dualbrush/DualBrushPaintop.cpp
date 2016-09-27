/*
  *  Copyright (c) 2016 Boudewijn Rempt <boud@valdyas.org>
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

#include "DualBrushPaintop.h"
#include "DualBrushPaintopSettings.h"

#include <cmath>
#include <QRect>

#include <KoColor.h>
#include <KoColorSpace.h>

#include <kis_image.h>
#include <kis_debug.h>

#include <kis_global.h>
#include <kis_paint_device.h>
#include <kis_painter.h>
#include <kis_types.h>
#include <brushengine/kis_paintop_registry.h>
#include <brushengine/kis_paintop.h>
#include <brushengine/kis_paint_information.h>

#include <kis_pressure_opacity_option.h>
#include <kis_lod_transform.h>
#include <DualBrushProperties.h>
#include <KoCompositeOpRegistry.h>
#include <kis_brush_based_paintop_settings.h>

#include "StackedPreset.h"

struct PainterConfigManager {
    PainterConfigManager(KisPainter *painter, const StackedPreset &preset) {
        m_painter = painter;
        m_oldOpacity = painter->opacity();
        m_oldCompositeOp = painter->compositeOp()->id();

        painter->setOpacity(m_oldOpacity * preset.opacity);
        painter->setCompositeOp(preset.compositeOp);
    }

    ~PainterConfigManager() {
        m_painter->setOpacity(m_oldOpacity);
        m_painter->setCompositeOp(m_oldCompositeOp);
    }

private:
    KisPainter *m_painter;
    quint8 m_oldOpacity;
    QString m_oldCompositeOp;
};

DualBrushPaintOp::DualBrushPaintOp(KisPaintOpSettingsSP settings, KisPainter *painter, KisNodeSP node, KisImageSP image)
    : KisPaintOp(painter)
{
    Q_UNUSED(image);
    Q_UNUSED(node);
    m_opacityOption.readOptionSetting(settings);
    m_opacityOption.resetAllSensors();


    DualBrushProperties prop;
    prop.readOptionSetting(settings.data());
    m_presetStack = prop.presetStack;

    m_paintopStack.clear();
    Q_FOREACH(const StackedPreset &preset, m_presetStack) {
        KisPaintOpSettingsSP settings = preset.paintopPreset->settings();
        KisBrushBasedPaintOpSettings *brushSettings = dynamic_cast<KisBrushBasedPaintOpSettings*>(settings.data());
        if (brushSettings) {
            brushSettings->setSpacing(preset.spacing);
            brushSettings->setAutoSpacing(false, 1.0);
            qDebug() << " YAY!!!!!!!"  << ppVar(preset.spacing);
        }
        settings->setPaintOpCompositeOp(preset.compositeOp);
        settings->setPaintOpOpacity(preset.opacity);

        KisPaintOp *paintop = KisPaintOpRegistry::instance()->paintOp(preset.paintopPreset, painter, node, image);
        m_paintopStack << paintop;
    }


    KIS_ASSERT_RECOVER_RETURN(!m_presetStack.isEmpty());
    m_distancesStore.resize(m_presetStack.size() - 1);
}

DualBrushPaintOp::~DualBrushPaintOp()
{
    qDeleteAll(m_paintopStack);
}

KisSpacingInformation DualBrushPaintOp::paintAt(const KisPaintInformation& info)
{
    if (!painter()) return KisSpacingInformation(1.0);

    KisSpacingInformation finalSpacing;

    for (int i = 0; i < m_paintopStack.size(); i++) {
        PainterConfigManager painterConfig(painter(), m_presetStack[i]);
        KisSpacingInformation spacing = m_paintopStack[i]->paintAt(info);

        if (!i) {
            finalSpacing = spacing;
        }
    }

    return finalSpacing;
}

void DualBrushPaintOp::paintLine(const KisPaintInformation &pi1, const KisPaintInformation &pi2, KisDistanceInformation *currentDistance)
{
    QVector<KisDistanceInformation*> distances = effectiveDistances(currentDistance);
    KIS_SAFE_ASSERT_RECOVER_RETURN(distances.size() == m_paintopStack.size());

    for (int i = 0; i < m_paintopStack.size(); i++) {
        PainterConfigManager painterConfig(painter(), m_presetStack[i]);
        m_paintopStack[i]->paintLine(pi1, pi2, distances[i]);
    }
}

void DualBrushPaintOp::paintBezierCurve(const KisPaintInformation &pi1, const QPointF &control1, const QPointF &control2, const KisPaintInformation &pi2, KisDistanceInformation *currentDistance)
{
    QVector<KisDistanceInformation*> distances = effectiveDistances(currentDistance);
    KIS_SAFE_ASSERT_RECOVER_RETURN(distances.size() == m_paintopStack.size());

    for (int i = 0; i < m_paintopStack.size(); i++) {
        PainterConfigManager painterConfig(painter(), m_presetStack[i]);
        m_paintopStack[i]->paintBezierCurve(pi1, control1, control2, pi2, distances[i]);
    }
}

QVector<KisDistanceInformation*> DualBrushPaintOp::effectiveDistances(KisDistanceInformation *mainDistance)
{
    QVector<KisDistanceInformation*> distances;
    distances << mainDistance;
    auto it = m_distancesStore.begin();
    auto end = m_distancesStore.end();

    for (; it != end; ++it) {
        distances << &(*it);
    }

    return distances;
}

