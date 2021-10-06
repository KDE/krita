/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2012 Boudewijn Rempt <boud@valdyas.org>
 * SPDX-FileCopyrightText: 2014 Mohit Goyal <mohit.bits2011@gmail.com>
 * SPDX-FileCopyrightText: 2021 Deif Lou <ginoba@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */
#include "kis_texture_option.h"

#include <QWidget>
#include <QString>
#include <QByteArray>
#include <QCheckBox>
#include <QBuffer>
#include <QFormLayout>
#include <QLabel>
#include <QComboBox>
#include <QTransform>
#include <QPainter>
#include <QBoxLayout>

#include <klocalizedstring.h>

#include <kis_pattern_chooser.h>
#include <kis_slider_spin_box.h>
#include <kis_multipliers_double_slider_spinbox.h>
#include <KoPattern.h>
#include <KoAbstractGradient.h>
#include <KoResource.h>
#include <KoResourceServerProvider.h>
#include <kis_paint_device.h>
#include <kis_fill_painter.h>
#include <kis_painter.h>
#include <kis_iterator_ng.h>
#include <kis_fixed_paint_device.h>
#include <KisLevelsSlider.h>
#include "kis_linked_pattern_manager.h"
#include <brushengine/kis_paintop_lod_limitations.h>
#include "kis_texture_chooser.h"
#include "KoMixColorsOp.h"
#include <time.h>
#include "kis_signals_blocker.h"
#include <KisGlobalResourcesInterface.h>
#include <strokes/KisMaskingBrushCompositeOpBase.h>
#include <strokes/KisMaskingBrushCompositeOpFactory.h>
#include <kis_random_accessor_ng.h>
#include <KoCompositeOpRegistry.h>

#include <KoCanvasResourcesIds.h>
#include <KoCanvasResourcesInterface.h>

KisTextureOption::KisTextureOption(KisBrushTextureFlags flags)
    : KisPaintOpOption(KisPaintOpOption::TEXTURE, true)
    , m_textureOptions(new KisTextureChooser(flags))
{
    setObjectName("KisTextureOption");
    setConfigurationPage(m_textureOptions);

    connect(m_textureOptions->textureSelectorWidget, SIGNAL(resourceSelected(KoResourceSP )), SLOT(resetGUI(KoResourceSP )));
    connect(m_textureOptions->textureSelectorWidget, SIGNAL(resourceSelected(KoResourceSP )), SLOT(emitSettingChanged()));
    connect(m_textureOptions->scaleSlider, SIGNAL(valueChanged(qreal)), SLOT(emitSettingChanged()));
    connect(m_textureOptions->brightnessSlider, SIGNAL(valueChanged(qreal)), SLOT(emitSettingChanged()));
    connect(m_textureOptions->contrastSlider, SIGNAL(valueChanged(qreal)), SLOT(emitSettingChanged()));
    connect(m_textureOptions->neutralPointSlider, SIGNAL(valueChanged(qreal)), SLOT(emitSettingChanged()));
    connect(m_textureOptions->offsetSliderX, SIGNAL(valueChanged(int)), SLOT(emitSettingChanged()));
    connect(m_textureOptions->randomOffsetX, SIGNAL(toggled(bool)), SLOT(emitSettingChanged()));
    connect(m_textureOptions->randomOffsetY, SIGNAL(toggled(bool)), SLOT(emitSettingChanged()));
    connect(m_textureOptions->offsetSliderY, SIGNAL(valueChanged(int)), SLOT(emitSettingChanged()));
    connect(m_textureOptions->cmbTexturingMode, SIGNAL(currentIndexChanged(int)), SLOT(emitSettingChanged()));
    connect(m_textureOptions->cmbCutoffPolicy, SIGNAL(currentIndexChanged(int)), SLOT(emitSettingChanged()));
    connect(m_textureOptions->cutoffSlider, SIGNAL(blackPointChanged(qreal)), SLOT(emitSettingChanged()));
    connect(m_textureOptions->cutoffSlider, SIGNAL(whitePointChanged(qreal)), SLOT(emitSettingChanged()));
    connect(m_textureOptions->chkInvert, SIGNAL(toggled(bool)), SLOT(emitSettingChanged()));
    resetGUI(m_textureOptions->textureSelectorWidget->currentResource());

}

KisTextureOption::~KisTextureOption()
{
    delete m_textureOptions;
}

void KisTextureOption::writeOptionSetting(KisPropertiesConfigurationSP setting) const
{
    KoPatternSP pattern;

    {
        KisSignalsBlocker b(m_textureOptions->textureSelectorWidget);
        KoResourceSP resource = m_textureOptions->textureSelectorWidget->currentResource();
        if (!resource) return;

        pattern = resource.staticCast<KoPattern>();
        if (!pattern) return;
    }

    setting->setProperty("Texture/Pattern/Enabled", isChecked());
    if (!isChecked()) {
        return;
    }

    qreal scale = m_textureOptions->scaleSlider->value();

    qreal brightness = m_textureOptions->brightnessSlider->value();

    qreal contrast = m_textureOptions->contrastSlider->value();

    qreal neutralPoint = m_textureOptions->neutralPointSlider->value();

    int offsetX = m_textureOptions->offsetSliderX->value();
    if (m_textureOptions ->randomOffsetX->isChecked()) {

        m_textureOptions->offsetSliderX ->setEnabled(false);
        m_textureOptions->offsetSliderX ->blockSignals(true);
        m_textureOptions->offsetSliderX ->setValue(offsetX);
        m_textureOptions->offsetSliderX ->blockSignals(false);
    }
    else {
        m_textureOptions->offsetSliderX ->setEnabled(true);
    }

    int offsetY = m_textureOptions->offsetSliderY->value();
    if (m_textureOptions ->randomOffsetY->isChecked()) {

        m_textureOptions->offsetSliderY ->setEnabled(false);
        m_textureOptions->offsetSliderY ->blockSignals(true);
        m_textureOptions->offsetSliderY ->setValue(offsetY);
        m_textureOptions->offsetSliderY ->blockSignals(false);
    }
    else {
        m_textureOptions->offsetSliderY ->setEnabled(true);
    }

    int texturingMode = m_textureOptions->texturingMode();
    bool invert = (m_textureOptions->chkInvert->checkState() == Qt::Checked);

    setting->setProperty("Texture/Pattern/Scale", scale);
    setting->setProperty("Texture/Pattern/Brightness", brightness);
    setting->setProperty("Texture/Pattern/Contrast", contrast);
    setting->setProperty("Texture/Pattern/NeutralPoint", neutralPoint);
    setting->setProperty("Texture/Pattern/OffsetX", offsetX);
    setting->setProperty("Texture/Pattern/OffsetY", offsetY);
    setting->setProperty("Texture/Pattern/TexturingMode", texturingMode);
    setting->setProperty("Texture/Pattern/CutoffLeft", static_cast<int>(m_textureOptions->cutoffSlider->blackPoint() * 255.0));
    setting->setProperty("Texture/Pattern/CutoffRight", static_cast<int>(m_textureOptions->cutoffSlider->whitePoint() * 255.0));
    setting->setProperty("Texture/Pattern/CutoffPolicy", m_textureOptions->cmbCutoffPolicy->currentIndex());
    setting->setProperty("Texture/Pattern/Invert", invert);

    setting->setProperty("Texture/Pattern/MaximumOffsetX",m_textureOptions->offsetSliderX ->maximum());
    setting->setProperty("Texture/Pattern/MaximumOffsetY",m_textureOptions->offsetSliderY ->maximum());
    setting->setProperty("Texture/Pattern/isRandomOffsetX",m_textureOptions ->randomOffsetX ->isChecked());
    setting->setProperty("Texture/Pattern/isRandomOffsetY",m_textureOptions ->randomOffsetY ->isChecked());

    KisLinkedPatternManager::saveLinkedPattern(setting, pattern);
}

void KisTextureOption::readOptionSetting(const KisPropertiesConfigurationSP setting)
{
    setChecked(setting->getBool("Texture/Pattern/Enabled"));

    if (!isChecked()) {
        return;
    }
    KoPatternSP pattern = KisLinkedPatternManager::loadLinkedPattern(setting, resourcesInterface());

    if (!pattern ){
        qWarning() << "Could not get linked pattern";
        pattern = m_textureOptions->textureSelectorWidget->currentResource().staticCast<KoPattern>();
    }
    m_textureOptions->textureSelectorWidget->setCurrentPattern(pattern);

    m_textureOptions->scaleSlider->setValue(setting->getDouble("Texture/Pattern/Scale", 1.0));
    m_textureOptions->brightnessSlider->setValue(setting->getDouble("Texture/Pattern/Brightness"));
    m_textureOptions->contrastSlider->setValue(setting->getDouble("Texture/Pattern/Contrast", 1.0));
    m_textureOptions->neutralPointSlider->setValue(setting->getDouble("Texture/Pattern/NeutralPoint", 0.5));
    m_textureOptions->offsetSliderX->setValue(setting->getInt("Texture/Pattern/OffsetX"));
    m_textureOptions->offsetSliderY->setValue(setting->getInt("Texture/Pattern/OffsetY"));
    m_textureOptions->randomOffsetX->setChecked(setting->getBool("Texture/Pattern/isRandomOffsetX"));
    m_textureOptions->randomOffsetY->setChecked(setting->getBool("Texture/Pattern/isRandomOffsetY"));
    m_textureOptions->selectTexturingMode(KisTextureProperties::TexturingMode(setting->getInt("Texture/Pattern/TexturingMode", KisTextureProperties::MULTIPLY)));
    m_textureOptions->cmbCutoffPolicy->setCurrentIndex(setting->getInt("Texture/Pattern/CutoffPolicy"));
    m_textureOptions->cutoffSlider->reset(static_cast<qreal>(setting->getInt("Texture/Pattern/CutoffLeft", 0)) / 255.0,
                                          static_cast<qreal>(setting->getInt("Texture/Pattern/CutoffRight", 255)) / 255.0);
    m_textureOptions->chkInvert->setChecked(setting->getBool("Texture/Pattern/Invert"));

}

void KisTextureOption::lodLimitations(KisPaintopLodLimitations *l) const
{
    l->limitations << KoID("texture-pattern", i18nc("PaintOp instant preview limitation", "Texture->Pattern (low quality preview)"));
}


void KisTextureOption::resetGUI(KoResourceSP res)
{
    KoPatternSP pattern = res.staticCast<KoPattern>();
    if (!pattern) return;

    m_textureOptions->offsetSliderX->setRange(0, pattern->pattern().width() / 2);
    m_textureOptions->offsetSliderY->setRange(0, pattern->pattern().height() / 2);
}

/**********************************************************************/
/*       KisTextureProperties                                         */
/**********************************************************************/


KisTextureProperties::KisTextureProperties(int levelOfDetail, KisBrushTextureFlags flags)
    : m_gradient(0)
    , m_levelOfDetail(levelOfDetail)
    , m_flags(flags)
    , m_enabled(false)
{
}

void KisTextureProperties::fillProperties(const KisPropertiesConfigurationSP setting, KisResourcesInterfaceSP resourcesInterface, KoCanvasResourcesInterfaceSP canvasResourcesInterface)
{
    if (setting->getString("Texture/Pattern/PatternMD5").isEmpty()) {
        m_enabled = false;
        return;
    }

    m_texturingMode = (TexturingMode)setting->getInt("Texture/Pattern/TexturingMode", MULTIPLY);

    if (!(m_flags & SupportsLightnessMode) && (m_texturingMode == LIGHTNESS)) {
        m_texturingMode = SUBTRACT;
    }

    if (!(m_flags & SupportsGradientMode) && (m_texturingMode == GRADIENT)) {
        m_texturingMode = SUBTRACT;
    }

    const bool preserveAlpha = m_texturingMode == LIGHTNESS || m_texturingMode == GRADIENT;

    m_maskInfo = toQShared(new KisTextureMaskInfo(m_levelOfDetail, preserveAlpha));
    if (!m_maskInfo->fillProperties(setting, resourcesInterface)) {
        warnKrita << "WARNING: Couldn't load the pattern for a stroke (KisTextureProperties)";
        m_enabled = false;
        return;
    }

    m_maskInfo = KisTextureMaskInfoCache::instance()->fetchCachedTextureInfo(m_maskInfo);

    m_enabled = setting->getBool("Texture/Pattern/Enabled", false);
    m_offsetX = setting->getInt("Texture/Pattern/OffsetX");
    m_offsetY = setting->getInt("Texture/Pattern/OffsetY");

    if (m_texturingMode == GRADIENT && canvasResourcesInterface) {
        KoAbstractGradientSP gradient = canvasResourcesInterface->resource(KoCanvasResource::CurrentGradient).value<KoAbstractGradientSP>()->cloneAndBakeVariableColors(canvasResourcesInterface);
        if (gradient) {
            m_gradient = gradient;
            m_cachedGradient.setGradient(gradient, 256);
        }
    }

    m_strengthOption.readOptionSetting(setting);
    m_strengthOption.resetAllSensors();
}

QList<KoResourceSP> KisTextureProperties::prepareEmbeddedResources(const KisPropertiesConfigurationSP setting, KisResourcesInterfaceSP resourcesInterface)
{
    /**
     * We cannot use m_enabled here because it is not initialized at this stage.
     * fillProperties() is not necessary for this call, becasue it is extremely slow.
     */

    QList<KoResourceSP> patterns;

    const bool enabled = setting->getBool("Texture/Pattern/Enabled", false);
    if (enabled) {
        KoPatternSP pattern = KisLinkedPatternManager::loadLinkedPattern(setting, resourcesInterface);
        if (pattern) {
            patterns << pattern;
        }
    }

    return patterns;
}

bool KisTextureProperties::applyingGradient() const
{
    return m_texturingMode == GRADIENT;
}

bool KisTextureProperties::applyingGradient(const KisPropertiesConfiguration *settings)
{
    return (TexturingMode) settings->getInt("Texture/Pattern/TexturingMode", MULTIPLY) == GRADIENT;
}

void KisTextureProperties::applyLightness(KisFixedPaintDeviceSP dab, const QPoint& offset, const KisPaintInformation& info) {
    if (!m_enabled) return;
    if (!m_maskInfo->isValid()) return;

    KisPaintDeviceSP mask = m_maskInfo->mask();
    const QRect maskBounds = m_maskInfo->maskBounds();

    KisPaintDeviceSP fillMaskDevice = new KisPaintDevice(KoColorSpaceRegistry::instance()->rgb8());
    const QRect rect = dab->bounds();

    int x = offset.x() % maskBounds.width() - m_offsetX;
    int y = offset.y() % maskBounds.height() - m_offsetY;

    KisFillPainter fillMaskPainter(fillMaskDevice);
    fillMaskPainter.fillRect(x - 1, y - 1, rect.width() + 2, rect.height() + 2, mask, maskBounds);
    fillMaskPainter.end();

    qreal pressure = m_strengthOption.apply(info);
    quint8* dabData = dab->data();

    KisSequentialConstIterator it(fillMaskDevice, QRect(x, y, rect.width(), rect.height()));
    while (it.nextPixel()) {
        const QRgb *maskQRgb = reinterpret_cast<const QRgb*>(it.oldRawData());
        dab->colorSpace()->fillGrayBrushWithColorAndLightnessWithStrength(dabData, maskQRgb, dabData, pressure, 1);
        dabData += dab->pixelSize();
    }
}

void KisTextureProperties::applyGradient(KisFixedPaintDeviceSP dab, const QPoint& offset, const KisPaintInformation& info) {
    if (!m_enabled) return;
    if (!m_maskInfo->isValid()) return;

    KIS_SAFE_ASSERT_RECOVER_RETURN(m_gradient && m_gradient->valid());

    KisPaintDeviceSP fillDevice = new KisPaintDevice(KoColorSpaceRegistry::instance()->rgb8());
    QRect rect = dab->bounds();

    KisPaintDeviceSP mask = m_maskInfo->mask();
    const QRect maskBounds = m_maskInfo->maskBounds();

    int x = offset.x() % maskBounds.width() - m_offsetX;
    int y = offset.y() % maskBounds.height() - m_offsetY;


    KisFillPainter fillPainter(fillDevice);
    fillPainter.fillRect(x - 1, y - 1, rect.width() + 2, rect.height() + 2, mask, maskBounds);
    fillPainter.end();

    qreal pressure = m_strengthOption.apply(info);
    quint8* dabData = dab->data();

    //for gradient textures...
    KoMixColorsOp* colorMix = dab->colorSpace()->mixColorsOp();
    qint16 colorWeights[2];
    colorWeights[0] = qRound(pressure * 255);
    colorWeights[1] = 255 - colorWeights[0];
    quint8* colors[2];
    m_cachedGradient.setColorSpace(dab->colorSpace()); //Change colorspace here so we don't have to convert each pixel drawn

    KisHLineIteratorSP iter = fillDevice->createHLineIteratorNG(x, y, rect.width());
    for (int row = 0; row < rect.height(); ++row) {
        for (int col = 0; col < rect.width(); ++col) {

            const QRgb* maskQRgb = reinterpret_cast<const QRgb*>(iter->oldRawData());
            qreal gradientvalue = qreal(qGray(*maskQRgb))/255.0;//qreal(*iter->oldRawData()) / 255.0;
            KoColor paintcolor;
            paintcolor.setColor(m_cachedGradient.cachedAt(gradientvalue), dab->colorSpace());
            qreal paintOpacity = paintcolor.opacityF() * (qreal(qAlpha(*maskQRgb)) / 255.0);
            paintcolor.setOpacity(qMin(paintOpacity, dab->colorSpace()->opacityF(dabData)));
            colors[0] = paintcolor.data();
            KoColor dabColor(dabData, dab->colorSpace());
            colors[1] = dabColor.data();
            colorMix->mixColors(colors, colorWeights, 2, dabData);

            iter->nextPixel();
            dabData += dab->pixelSize();
        }
        iter->nextRow();
    }
}

void KisTextureProperties::apply(KisFixedPaintDeviceSP dab, const QPoint &offset, const KisPaintInformation & info)
{
    if (!m_enabled) return;
    if (!m_maskInfo->isValid()) return;

    if (m_texturingMode == LIGHTNESS) {
        applyLightness(dab, offset, info);
        return;
    }
    else if (m_texturingMode == GRADIENT && m_gradient) {
        applyGradient(dab, offset, info);
        return;
    }

    // Create mask device
    KisPaintDeviceSP maskPatch = new KisPaintDevice(KoColorSpaceRegistry::instance()->alpha8());
    QRect rect = dab->bounds();

    KisPaintDeviceSP mask = m_maskInfo->mask();
    const QRect maskBounds = m_maskInfo->maskBounds();

    int x = offset.x() % maskBounds.width() - m_offsetX;
    int y = offset.y() % maskBounds.height() - m_offsetY;

    KisFillPainter fillPainter(maskPatch);
    fillPainter.fillRect(x - 1, y - 1, rect.width() + 2, rect.height() + 2, mask, maskBounds);
    fillPainter.end();

    // Compute final strength
    qreal strength = m_strengthOption.apply(info);

    // Select mask compositing op
    KoChannelInfo::enumChannelValueType alphaChannelType = KoChannelInfo::UINT8;
    int alphaChannelOffset = -1;

    QList<KoChannelInfo *> channels = dab->colorSpace()->channels();
    for (quint32 i = 0; i < dab->pixelSize(); i++) {
        if (channels[i]->channelType() == KoChannelInfo::ALPHA) {
            // TODO: check correctness for 16bits!
            alphaChannelOffset = channels[i]->pos()/* * channels[i]->size()*/;
            alphaChannelType = channels[i]->channelValueType();
            break;
        }
    }

    KIS_SAFE_ASSERT_RECOVER (alphaChannelOffset >= 0) {
        alphaChannelOffset = 0;
    }

    QScopedPointer<KisMaskingBrushCompositeOpBase> compositeOp;

    switch (m_texturingMode) {
    case MULTIPLY:
        compositeOp.reset(KisMaskingBrushCompositeOpFactory::createForAlphaSrc(COMPOSITE_MULT, alphaChannelType, dab->pixelSize(), alphaChannelOffset, strength));
        break;
    case SUBTRACT:
        compositeOp.reset(KisMaskingBrushCompositeOpFactory::createForAlphaSrc(COMPOSITE_SUBTRACT, alphaChannelType, dab->pixelSize(), alphaChannelOffset, strength));
        break;
    case DARKEN:
        compositeOp.reset(KisMaskingBrushCompositeOpFactory::createForAlphaSrc(COMPOSITE_DARKEN, alphaChannelType, dab->pixelSize(), alphaChannelOffset, strength));
        break;
    case OVERLAY:
        compositeOp.reset(KisMaskingBrushCompositeOpFactory::createForAlphaSrc(COMPOSITE_OVERLAY, alphaChannelType, dab->pixelSize(), alphaChannelOffset, strength));
        break;
    case COLOR_DODGE:
        compositeOp.reset(KisMaskingBrushCompositeOpFactory::createForAlphaSrc(COMPOSITE_DODGE, alphaChannelType, dab->pixelSize(), alphaChannelOffset, strength));
        break;
    case COLOR_BURN:
        compositeOp.reset(KisMaskingBrushCompositeOpFactory::createForAlphaSrc(COMPOSITE_BURN, alphaChannelType, dab->pixelSize(), alphaChannelOffset, strength));
        break;
    case LINEAR_DODGE:
        compositeOp.reset(KisMaskingBrushCompositeOpFactory::createForAlphaSrc(COMPOSITE_LINEAR_DODGE, alphaChannelType, dab->pixelSize(), alphaChannelOffset, strength));
        break;
    case LINEAR_BURN:
        compositeOp.reset(KisMaskingBrushCompositeOpFactory::createForAlphaSrc(COMPOSITE_LINEAR_BURN, alphaChannelType, dab->pixelSize(), alphaChannelOffset, strength));
        break;
    case HARD_MIX_PHOTOSHOP:
        compositeOp.reset(KisMaskingBrushCompositeOpFactory::createForAlphaSrc(COMPOSITE_HARD_MIX_PHOTOSHOP, alphaChannelType, dab->pixelSize(), alphaChannelOffset, strength));
        break;
    case HARD_MIX_SOFTER_PHOTOSHOP:
        compositeOp.reset(KisMaskingBrushCompositeOpFactory::createForAlphaSrc(COMPOSITE_HARD_MIX_SOFTER_PHOTOSHOP, alphaChannelType, dab->pixelSize(), alphaChannelOffset, strength));
        break;
    case HEIGHT:
        compositeOp.reset(KisMaskingBrushCompositeOpFactory::createForAlphaSrc("height", alphaChannelType, dab->pixelSize(), alphaChannelOffset, strength));
        break;
    case LINEAR_HEIGHT:
        compositeOp.reset(KisMaskingBrushCompositeOpFactory::createForAlphaSrc("linear_height", alphaChannelType, dab->pixelSize(), alphaChannelOffset, strength));
        break;
    case HEIGHT_PHOTOSHOP:
        compositeOp.reset(KisMaskingBrushCompositeOpFactory::createForAlphaSrc("height_photoshop", alphaChannelType, dab->pixelSize(), alphaChannelOffset, strength));
        break;
    case LINEAR_HEIGHT_PHOTOSHOP:
        compositeOp.reset(KisMaskingBrushCompositeOpFactory::createForAlphaSrc("linear_height_photoshop", alphaChannelType, dab->pixelSize(), alphaChannelOffset, strength));
        break;
    default: return;
    }

    // Apply the mask to the dab
    {
        quint8 *dabIt = nullptr;
        KisRandomConstAccessorSP maskPatchIt = maskPatch->createRandomConstAccessorNG();

        qint32 dabY = dab->bounds().y();
        qint32 maskPatchY = maskPatch->exactBounds().y() + 1;
        qint32 rowsRemaining = dab->bounds().height();
        const qint32 dabRowStride = dab->bounds().width() * dab->pixelSize();

        while (rowsRemaining > 0) {
            qint32 dabX = dab->bounds().x();
            qint32 maskPatchX = maskPatch->exactBounds().x() + 1;
            const qint32 numContiguousMaskPatchRows = maskPatchIt->numContiguousRows(maskPatchY);
            const qint32 rows = std::min(rowsRemaining, numContiguousMaskPatchRows);
            qint32 columnsRemaining = dab->bounds().width();

            while (columnsRemaining > 0) {
                const qint32 numContiguousMaskPatchColumns = maskPatchIt->numContiguousColumns(maskPatchX);
                const qint32 columns = std::min(columnsRemaining, numContiguousMaskPatchColumns);

                const qint32 maskPatchRowStride = maskPatchIt->rowStride(maskPatchX, maskPatchY);

                dabIt = dab->data() + (dabY * dab->bounds().width() + dabX) * dab->pixelSize();
                maskPatchIt->moveTo(maskPatchX, maskPatchY);

                compositeOp->composite(maskPatchIt->rawDataConst(), maskPatchRowStride,
                                       dabIt, dabRowStride,
                                       columns, rows);

                dabX += columns;
                maskPatchX += columns;
                columnsRemaining -= columns;

            }

            dabY += rows;
            maskPatchY += rows;
            rowsRemaining -= rows;
        }
    }
}
