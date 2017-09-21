/*
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
 *  Copyright (c) 2004-2008 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2004 Clarence Dang <dang@kde.org>
 *  Copyright (c) 2004 Adrian Page <adrian@pagenet.plus.com>
 *  Copyright (c) 2004 Cyrille Berger <cberger@cberger.net>
 *  Copyright (c) 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
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

#include "kis_brushop.h"

#include <QRect>

#include <kis_image.h>
#include <kis_vec.h>
#include <kis_debug.h>

#include <KoColorTransformation.h>
#include <KoColor.h>

#include <kis_brush.h>
#include <kis_global.h>
#include <kis_paint_device.h>
#include <kis_painter.h>
#include <kis_brush_based_paintop_settings.h>
#include <kis_lod_transform.h>
#include <kis_paintop_plugin_utils.h>
#include "krita_utils.h"
#include <QtConcurrent>
#include "kis_algebra_2d.h"
#include <KisDabRenderingExecutor.h>
#include <KisDabCacheUtils.h>
#include <KisRenderedDab.h>
#include "KisBrushOpResources.h"


KisBrushOp::KisBrushOp(const KisPaintOpSettingsSP settings, KisPainter *painter, KisNodeSP node, KisImageSP image)
    : KisBrushBasedPaintOp(settings, painter)
    , m_opacityOption(node)
{
    Q_UNUSED(image);
    Q_ASSERT(settings);

    m_airbrushOption.readOptionSetting(settings);

    m_opacityOption.readOptionSetting(settings);
    m_flowOption.readOptionSetting(settings);
    m_sizeOption.readOptionSetting(settings);
    m_ratioOption.readOptionSetting(settings);
    m_spacingOption.readOptionSetting(settings);
    m_rateOption.readOptionSetting(settings);
    m_softnessOption.readOptionSetting(settings);
    m_rotationOption.readOptionSetting(settings);
    m_scatterOption.readOptionSetting(settings);
    m_sharpnessOption.readOptionSetting(settings);

    m_opacityOption.resetAllSensors();
    m_flowOption.resetAllSensors();
    m_sizeOption.resetAllSensors();
    m_ratioOption.resetAllSensors();
    m_rateOption.resetAllSensors();
    m_softnessOption.resetAllSensors();
    m_sharpnessOption.resetAllSensors();
    m_rotationOption.resetAllSensors();
    m_scatterOption.resetAllSensors();
    m_sharpnessOption.resetAllSensors();

    m_rotationOption.applyFanCornersInfo(this);

    KisBrushSP baseBrush = m_brush;
    auto resourcesFactory =
        [baseBrush, settings, painter] () {
            KisDabCacheUtils::DabRenderingResources *resources =
                new KisBrushOpResources(settings, painter);
            resources->brush = baseBrush->clone();

            return resources;
        };

    m_dabExecutor.reset(
        new KisDabRenderingExecutor(
                    painter->device()->compositionSourceColorSpace(),
                    resourcesFactory,
                    &m_mirrorOption,
                    &m_precisionOption));
}

KisBrushOp::~KisBrushOp()
{
}

KisSpacingInformation KisBrushOp::paintAt(const KisPaintInformation& info)
{
    if (!painter()->device()) return KisSpacingInformation(1.0);

    KisBrushSP brush = m_brush;
    Q_ASSERT(brush);
    if (!brush)
        return KisSpacingInformation(1.0);

    if (!brush->canPaintFor(info))
        return KisSpacingInformation(1.0);

    qreal scale = m_sizeOption.apply(info);
    scale *= KisLodTransform::lodToScale(painter()->device());
    if (checkSizeTooSmall(scale)) return KisSpacingInformation();

    qreal rotation = m_rotationOption.apply(info);
    qreal ratio = m_ratioOption.apply(info);

    KisDabShape shape(scale, ratio, rotation);
    QPointF cursorPos =
        m_scatterOption.apply(info,
                              brush->maskWidth(shape, 0, 0, info),
                              brush->maskHeight(shape, 0, 0, info));

    m_opacityOption.setFlow(m_flowOption.apply(info));

    quint8 dabOpacity = OPACITY_OPAQUE_U8;
    quint8 dabFlow = OPACITY_OPAQUE_U8;

    m_opacityOption.apply(info, &dabOpacity, &dabFlow);

    KisDabCacheUtils::DabRequestInfo request(painter()->paintColor(),
                                             cursorPos,
                                             shape,
                                             info,
                                             m_softnessOption.apply(info));

    m_dabExecutor->addDab(request, qreal(dabOpacity) / 255.0, qreal(dabFlow) / 255.0);

    return effectiveSpacing(scale, rotation, &m_airbrushOption, &m_spacingOption, info);
}

int KisBrushOp::doAsyncronousUpdate(bool forceLastUpdate)
{
    if (forceLastUpdate) {
        m_dabExecutor->waitForDone();
    }

    if (!m_dabExecutor->hasPreparedDabs()) return m_currentUpdatePeriod;

    const int numThreads = 8;
    const int splitInto = numThreads;

    QList<KisRenderedDab> dabsQueue = m_dabExecutor->takeReadyDabs();
    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(!dabsQueue.isEmpty(), m_currentUpdatePeriod);

    QRect totalRect;
    Q_FOREACH (const KisRenderedDab &dab, dabsQueue) {
        totalRect |= dab.realBounds();
    }

    int idealPatchSize = KisAlgebra2D::maxDimension(totalRect) / splitInto;
    idealPatchSize &= ~63;

    idealPatchSize = qMax(idealPatchSize, 64);

    QVector<QRect> rects = KritaUtils::splitRectIntoPatches(totalRect, QSize(idealPatchSize,idealPatchSize));

    KisPainter *gc = painter();

    //ENTER_FUNCTION() << ppVar(idealPatchSize) << ppVar(rects.size()) << ppVar(dabsQueue.size());

    QtConcurrent::blockingMap(rects,
        [gc, dabsQueue] (const QRect &rc) {
             KisPainter painter(gc->device());
             painter.bltFixed(rc, dabsQueue);
        });

    Q_FOREACH(const QRect &rc, rects) {
        painter()->addDirtyRect(rc);
    }

    painter()->setAverageOpacity(dabsQueue.last().averageOpacity);

    const int dabRenderingTime = m_dabExecutor->averageDabRenderingTime();
    m_currentUpdatePeriod = qBound(20.0, 100.0 * dabRenderingTime, 80.0);

    return m_currentUpdatePeriod;
}


KisSpacingInformation KisBrushOp::updateSpacingImpl(const KisPaintInformation &info) const
{
    const qreal scale = m_sizeOption.apply(info) * KisLodTransform::lodToScale(painter()->device());
    qreal rotation = m_rotationOption.apply(info);
    return effectiveSpacing(scale, rotation, &m_airbrushOption, &m_spacingOption, info);
}

KisTimingInformation KisBrushOp::updateTimingImpl(const KisPaintInformation &info) const
{
    return KisPaintOpPluginUtils::effectiveTiming(&m_airbrushOption, &m_rateOption, info);
}

void KisBrushOp::paintLine(const KisPaintInformation& pi1, const KisPaintInformation& pi2, KisDistanceInformation *currentDistance)
{
    if (m_sharpnessOption.isChecked() && m_brush && (m_brush->width() == 1) && (m_brush->height() == 1)) {

        if (!m_lineCacheDevice) {
            m_lineCacheDevice = source()->createCompositionSourceDevice();
        }
        else {
            m_lineCacheDevice->clear();
        }

        KisPainter p(m_lineCacheDevice);
        p.setPaintColor(painter()->paintColor());
        p.drawDDALine(pi1.pos(), pi2.pos());

        QRect rc = m_lineCacheDevice->extent();
        painter()->bitBlt(rc.x(), rc.y(), m_lineCacheDevice, rc.x(), rc.y(), rc.width(), rc.height());
    //fixes Bug 338011
    painter()->renderMirrorMask(rc, m_lineCacheDevice);
    }
    else {
        KisPaintOp::paintLine(pi1, pi2, currentDistance);
    }
}
