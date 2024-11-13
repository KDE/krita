/*
 *  SPDX-FileCopyrightText: 2008-2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_hairy_paintop.h"
#include "kis_hairy_paintop_settings.h"

#include <cmath>
#include <QRect>

#include <kis_image.h>
#include <kis_debug.h>

#include "kis_paint_device.h"
#include "kis_painter.h"
#include <kis_vec.h>

#include "KisHairyInkOptionData.h"
#include "KisHairyBristleOptionData.h"
#include <kis_brush_option.h>
#include <kis_brush_based_paintop_settings.h>
#include <kis_fixed_paint_device.h>
#include <kis_lod_transform.h>
#include <kis_spacing_information.h>
#include <KoResourceLoadResult.h>


#include "kis_brush.h"

KisHairyPaintOp::KisHairyPaintOp(const KisPaintOpSettingsSP settings, KisPainter * painter, KisNodeSP node, KisImageSP image)
    : KisPaintOp(painter)
    , m_opacityOption(settings.data())
    , m_sizeOption(settings.data())
    , m_rotationOption(settings.data())
{
    Q_UNUSED(image);
    Q_ASSERT(settings);

    m_hairyBristleOption.read(settings.data());
    m_hairyInkOption.read(settings.data());

    m_dev = node ? node->paintDevice() : 0;

    KisBrushOptionProperties brushOption;
    brushOption.readOptionSetting(settings, settings->resourcesInterface(), settings->canvasResourcesInterface());
    KisBrushSP brush = brushOption.brush();
    KisFixedPaintDeviceSP dab = cachedDab(painter->device()->compositionSourceColorSpace());

    // properly initialize fake paint information to avoid warnings
    KisPaintInformation fakePaintInformation;
    fakePaintInformation.setRandomSource(new KisRandomSource());
    fakePaintInformation.setPerStrokeRandomSource(new KisPerStrokeRandomSource());

    if (brush->brushApplication() == IMAGESTAMP) {
        dab = brush->paintDevice(source()->colorSpace(), KisDabShape(), fakePaintInformation);
    } else {
        brush->mask(dab, painter->paintColor(), KisDabShape(), fakePaintInformation);
    }

    m_brush.fromDabWithDensity(dab, m_hairyBristleOption.densityFactor * 0.01);
    m_brush.setInkColor(painter->paintColor());

    loadSettings();
    m_brush.setProperties(&m_properties);
}

QList<KoResourceLoadResult> KisHairyPaintOp::prepareLinkedResources(const KisPaintOpSettingsSP settings, KisResourcesInterfaceSP resourcesInterface)
{
    KisBrushOptionProperties brushOption;
    return brushOption.prepareLinkedResources(settings, resourcesInterface);
}

void KisHairyPaintOp::loadSettings()
{
    m_properties.inkAmount = m_hairyInkOption.inkAmount;
    //TODO: wait for the transfer function with variable size

    m_properties.inkDepletionCurve = KisCubicCurve(m_hairyInkOption.inkDepletionCurve).floatTransfer(m_hairyInkOption.inkAmount);

    m_properties.inkDepletionEnabled = m_hairyInkOption.inkDepletionEnabled;
    m_properties.useSaturation = m_hairyInkOption.useSaturation;
    m_properties.useOpacity = m_hairyInkOption.useOpacity;
    m_properties.useWeights = m_hairyInkOption.useWeights;

    m_properties.pressureWeight = m_hairyInkOption.pressureWeight / 100.0;
    m_properties.bristleLengthWeight = m_hairyInkOption.bristleLengthWeight / 100.0;
    m_properties.bristleInkAmountWeight = m_hairyInkOption.bristleInkAmountWeight / 100.0;
    m_properties.inkDepletionWeight = m_hairyInkOption.inkDepletionWeight;
    m_properties.useSoakInk = m_hairyInkOption.useSoakInk;

    m_properties.useMousePressure = m_hairyBristleOption.useMousePressure;
    m_properties.shearFactor = m_hairyBristleOption.shearFactor;
    m_properties.randomFactor = m_hairyBristleOption.randomFactor;
    m_properties.scaleFactor = m_hairyBristleOption.scaleFactor;
    m_properties.threshold = m_hairyBristleOption.threshold;
    m_properties.antialias = m_hairyBristleOption.antialias;
    m_properties.useCompositing = m_hairyBristleOption.useCompositing;
    m_properties.connectedPath = m_hairyBristleOption.connectedPath;
}


KisSpacingInformation KisHairyPaintOp::paintAt(const KisPaintInformation& info)
{
    return updateSpacingImpl(info);
}

KisSpacingInformation KisHairyPaintOp::updateSpacingImpl(const KisPaintInformation &info) const
{
    Q_UNUSED(info);
    return KisSpacingInformation(0.5);
}


void KisHairyPaintOp::paintLine(const KisPaintInformation &pi1, const KisPaintInformation &pi2, KisDistanceInformation *currentDistance)
{
    Q_UNUSED(currentDistance);
    if (!painter()) return;

    if (!m_dab) {
        m_dab = source()->createCompositionSourceDevice();
    }
    else {
        m_dab->clear();
    }

    /**
     * Even though we don't use spacing in hairy brush, we should still
     * initialize its distance information to ensure drawing angle and
     * other history-based sensors work fine.
     */
    KisPaintInformation pi(pi2);
    KisPaintInformation::DistanceInformationRegistrar r =
        pi.registerDistanceInformation(currentDistance);

    // Hairy Brush is capable of working with zero scale,
    // so no additional checks for 'zero'ness are needed
    qreal scale = m_sizeOption.apply(pi);
    scale *= KisLodTransform::lodToScale(painter()->device());
    qreal rotation = m_rotationOption.apply(pi);
    qreal origOpacity = m_opacityOption.apply(painter(), pi);

    const bool mirrorFlip = pi1.canvasMirroredH() != pi1.canvasMirroredV();

    // we don't use spacing here (the brush itself is used only once
    // during initialization), so we should just skip the distance info
    // update

    m_brush.paintLine(m_dab, m_dev, pi1, pi, scale * m_hairyBristleOption.scaleFactor, mirrorFlip ? -rotation : rotation);

    //QRect rc = m_dab->exactBounds();
    QRect rc = m_dab->extent();
    painter()->bitBlt(rc.topLeft(), m_dab, rc);
    painter()->renderMirrorMask(rc, m_dab);
    painter()->setOpacityF(origOpacity);

    // we don't use spacing in hairy brush, but history is
    // still important for us
    currentDistance->registerPaintedDab(pi,
                                        KisSpacingInformation(),
                                        KisTimingInformation());
}
