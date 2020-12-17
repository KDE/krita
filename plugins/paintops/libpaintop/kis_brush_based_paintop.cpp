/*
 *  SPDX-FileCopyrightText: 2008 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "kis_brush_based_paintop.h"
#include "kis_properties_configuration.h"
#include <brushengine/kis_paintop_settings.h>
#include "kis_brush_option.h"
#include <kis_pressure_spacing_option.h>
#include <kis_pressure_rate_option.h>
#include "kis_painter.h"
#include <kis_lod_transform.h>
#include "kis_paintop_utils.h"
#include "kis_paintop_plugin_utils.h"

#include <QGlobalStatic>

#include <QImage>
#include <QPainter>

#ifdef HAVE_THREADED_TEXT_RENDERING_WORKAROUND

Q_GLOBAL_STATIC(TextBrushInitializationWorkaround, s_instance)


TextBrushInitializationWorkaround *TextBrushInitializationWorkaround::instance()
{
    return s_instance;
}

void TextBrushInitializationWorkaround::preinitialize(KisPaintOpSettingsSP settings)
{
    if (KisBrushOptionProperties::isTextBrush(settings.data())) {
        KisBrushOptionProperties brushOption;
        brushOption.readOptionSetting(settings, settings->resourcesInterface(), settings->canvasResourcesInterface());
        m_brush = brushOption.brush();
        m_settings = settings;
    }
    else {
        m_brush = 0;
        m_settings = 0;
    }
}

KisBrushSP TextBrushInitializationWorkaround::tryGetBrush(const KisPropertiesConfigurationSP settings)
{
    return (settings && settings == m_settings ? m_brush : 0);
}

TextBrushInitializationWorkaround::TextBrushInitializationWorkaround()
    : m_settings(0)
{}

TextBrushInitializationWorkaround::~TextBrushInitializationWorkaround()
{}

void KisBrushBasedPaintOp::preinitializeOpStatically(KisPaintOpSettingsSP settings)
{
    TextBrushInitializationWorkaround::instance()->preinitialize(settings);
}

#endif /* HAVE_THREADED_TEXT_RENDERING_WORKAROUND */


KisBrushBasedPaintOp::KisBrushBasedPaintOp(const KisPaintOpSettingsSP settings, KisPainter* painter)
    : KisPaintOp(painter),
      m_textureProperties(painter->device()->defaultBounds()->currentLevelOfDetail())
{
    Q_ASSERT(settings);

#ifdef HAVE_THREADED_TEXT_RENDERING_WORKAROUND
    m_brush =
            TextBrushInitializationWorkaround::instance()->tryGetBrush(settings);
#endif /* HAVE_THREADED_TEXT_RENDERING_WORKAROUND */

    if (!m_brush) {
        KisBrushOptionProperties brushOption;
        brushOption.readOptionSetting(settings, settings->resourcesInterface(), settings->canvasResourcesInterface());
        m_brush = brushOption.brush();
    }

    m_brush->notifyStrokeStarted();

    m_precisionOption.readOptionSetting(settings);
    m_dabCache = new KisDabCache(m_brush);
    m_dabCache->setPrecisionOption(&m_precisionOption);

    m_mirrorOption.readOptionSetting(settings);
    m_dabCache->setMirrorPostprocessing(&m_mirrorOption);

    m_textureProperties.fillProperties(settings, settings->resourcesInterface(), settings->canvasResourcesInterface());
    m_dabCache->setTexturePostprocessing(&m_textureProperties);

    m_precisionOption.setHasImprecisePositionOptions(
        m_precisionOption.hasImprecisePositionOptions() |
        m_mirrorOption.isChecked() | m_textureProperties.m_enabled);
}

KisBrushBasedPaintOp::~KisBrushBasedPaintOp()
{
    delete m_dabCache;
}

QList<KoResourceSP> KisBrushBasedPaintOp::prepareLinkedResources(const KisPaintOpSettingsSP settings, KisResourcesInterfaceSP resourcesInterface)
{
    QList<KoResourceSP> resources;

    KisBrushOptionProperties brushOption;
    resources << brushOption.prepareLinkedResources(settings, resourcesInterface);

    return resources;
}

QList<KoResourceSP> KisBrushBasedPaintOp::prepareEmbeddedResources(const KisPaintOpSettingsSP settings, KisResourcesInterfaceSP resourcesInterface)
{
    QList<KoResourceSP> resources;

    KisTextureProperties textureProperties(0);
    resources << textureProperties.prepareEmbeddedResources(settings, resourcesInterface);

    return resources;
}

bool KisBrushBasedPaintOp::checkSizeTooSmall(qreal scale)
{
    scale *= m_brush->scale();
    return KisPaintOpUtils::checkSizeTooSmall(scale, m_brush->width(), m_brush->height());
}

KisSpacingInformation KisBrushBasedPaintOp::effectiveSpacing(qreal scale) const
{
    // we parse dab rotation separately, so don't count it
    QSizeF metric = m_brush->characteristicSize(KisDabShape(scale, 1.0, 0));
    return effectiveSpacing(metric.width(), metric.height(), 1.0, false, 0.0, false);
}

KisSpacingInformation KisBrushBasedPaintOp::effectiveSpacing(qreal scale, qreal rotation, const KisPaintInformation &pi) const
{
    return effectiveSpacing(scale, rotation, nullptr, nullptr, pi);
}

KisSpacingInformation KisBrushBasedPaintOp::effectiveSpacing(qreal scale, qreal rotation, const KisPressureSpacingOption &spacingOption, const KisPaintInformation &pi) const
{
    return effectiveSpacing(scale, rotation, nullptr, &spacingOption, pi);
}

KisSpacingInformation KisBrushBasedPaintOp::effectiveSpacing(qreal scale, qreal rotation,
                                                             const KisAirbrushOptionProperties *airbrushOption,
                                                             const KisPressureSpacingOption *spacingOption,
                                                             const KisPaintInformation &pi) const
{
    bool isotropicSpacing = spacingOption && spacingOption->isotropicSpacing();

    MirrorProperties prop = m_mirrorOption.apply(pi);
    const bool implicitFlipped = prop.horizontalMirror != prop.verticalMirror;

    // we parse dab rotation separately, so don't count it
    QSizeF metric = m_brush->characteristicSize(KisDabShape(scale, 1.0, 0));

    return KisPaintOpPluginUtils::effectiveSpacing(metric.width(), metric.height(),
                                                   isotropicSpacing, rotation, implicitFlipped,
                                                   m_brush->spacing(),
                                                   m_brush->autoSpacingActive(),
                                                   m_brush->autoSpacingCoeff(),
                                                   KisLodTransform::lodToScale(painter()->device()),
                                                   airbrushOption, spacingOption,
                                                   pi);
}

KisSpacingInformation KisBrushBasedPaintOp::effectiveSpacing(qreal dabWidth, qreal dabHeight,
                                                             qreal extraScale,
                                                             bool isotropicSpacing, qreal rotation,
                                                             bool axesFlipped) const
{
    return KisPaintOpUtils::effectiveSpacing(dabWidth, dabHeight,
                                             extraScale,
                                             true,
                                             isotropicSpacing,
                                             rotation,
                                             axesFlipped,
                                             m_brush->spacing(),
                                             m_brush->autoSpacingActive(),
                                             m_brush->autoSpacingCoeff(),
                                             KisLodTransform::lodToScale(painter()->device()));
}

bool KisBrushBasedPaintOp::canPaint() const
{
    return m_brush != 0;
}
