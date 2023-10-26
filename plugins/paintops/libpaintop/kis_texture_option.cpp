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

#include <KoPattern.h>
#include <KoAbstractGradient.h>
#include <KoResource.h>
#include <KoResourceServerProvider.h>
#include <kis_paint_device.h>
#include <kis_fill_painter.h>
#include <kis_painter.h>
#include <kis_iterator_ng.h>
#include <kis_fixed_paint_device.h>
#include "KoMixColorsOp.h"
#include <strokes/KisMaskingBrushCompositeOpBase.h>
#include <strokes/KisMaskingBrushCompositeOpFactory.h>
#include <kis_random_accessor_ng.h>
#include <KoCompositeOpRegistry.h>

#include <KoCanvasResourcesIds.h>
#include <KoCanvasResourcesInterface.h>
#include <KoResourceLoadResult.h>

/**********************************************************************/
/*       KisTextureOption                                             */
/**********************************************************************/


KisTextureOption::KisTextureOption(const KisPropertiesConfiguration *setting,
                                   KisResourcesInterfaceSP resourcesInterface,
                                   KoCanvasResourcesInterfaceSP canvasResourcesInterface,
                                   int levelOfDetail,
                                   KisBrushTextureFlags flags)
    : m_gradient(0)
    , m_levelOfDetail(levelOfDetail)
    , m_strengthOption(setting)
    , m_flags(flags)
{
    fillProperties(setting, resourcesInterface, canvasResourcesInterface);
}

void KisTextureOption::fillProperties(const KisPropertiesConfiguration *setting, KisResourcesInterfaceSP resourcesInterface, KoCanvasResourcesInterfaceSP canvasResourcesInterface)
{
    KisTextureOptionData data;
    data.read(setting);

    if (data.textureData.isNull()) {
        m_enabled = false;
        return;
    }

    m_texturingMode = data.texturingMode;

    if (!(m_flags & SupportsLightnessMode) && (m_texturingMode == KisTextureOptionData::LIGHTNESS)) {
        m_texturingMode = KisTextureOptionData::SUBTRACT;
    }

    if (!(m_flags & SupportsGradientMode) && (m_texturingMode == KisTextureOptionData::GRADIENT)) {
        m_texturingMode = KisTextureOptionData::SUBTRACT;
    }

    const bool preserveAlpha = m_texturingMode == KisTextureOptionData::LIGHTNESS || m_texturingMode == KisTextureOptionData::GRADIENT;

    m_maskInfo = toQShared(new KisTextureMaskInfo(m_levelOfDetail, preserveAlpha));
    if (!m_maskInfo->fillProperties(setting, resourcesInterface)) {
        warnKrita << "WARNING: Couldn't load the pattern for a stroke (KisTextureProperties)";
        m_enabled = false;
        return;
    }

    m_maskInfo = KisTextureMaskInfoCache::instance()->fetchCachedTextureInfo(m_maskInfo);

    m_enabled = data.isEnabled;
    m_offsetX = data.offsetX;
    m_offsetY = data.offsetY;

    if (m_texturingMode == KisTextureOptionData::GRADIENT && canvasResourcesInterface) {
        KoAbstractGradientSP gradient = canvasResourcesInterface->resource(KoCanvasResource::CurrentGradient).value<KoAbstractGradientSP>()->cloneAndBakeVariableColors(canvasResourcesInterface);
        if (gradient) {
            m_gradient = gradient;
            m_cachedGradient.setGradient(gradient, 256);
        }
    }
}

QList<KoResourceLoadResult> KisTextureOption::prepareEmbeddedResources(const KisPropertiesConfigurationSP setting, KisResourcesInterfaceSP resourcesInterface)
{
    /**
     * We cannot use m_enabled here because it is not initialized at this stage.
     * fillProperties() is not necessary for this call, because it is extremely slow.
     */

    QList<KoResourceLoadResult> patterns;

    KisTextureOptionData data;
    data.read(setting.data());

    if (data.isEnabled && !data.textureData.patternBase64.isEmpty()) {
        patterns << data.textureData.loadLinkedPattern(resourcesInterface);
    }

    return patterns;
}

QList<KoResourceLoadResult> KisTextureOption::prepareLinkedResources(const KisPropertiesConfigurationSP setting, KisResourcesInterfaceSP resourcesInterface)
{
    /**
     * We cannot use m_enabled here because it is not initialized at this stage.
     * fillProperties() is not necessary for this call, because it is extremely slow.
     */

    QList<KoResourceLoadResult> patterns;

    KisTextureOptionData data;
    data.read(setting.data());

    if (data.isEnabled && data.textureData.patternBase64.isEmpty()) {
        patterns << data.textureData.loadLinkedPattern(resourcesInterface);
    }

    return patterns;
}

bool KisTextureOption::applyingGradient() const
{
    return m_texturingMode == KisTextureOptionData::GRADIENT;
}

bool KisTextureOption::applyingGradient(const KisPropertiesConfiguration *settings)
{
    KisTextureOptionData data;
    data.read(settings);

    return data.texturingMode == KisTextureOptionData::GRADIENT;
}

void KisTextureOption::applyLightness(KisFixedPaintDeviceSP dab, const QPoint& offset, const KisPaintInformation& info) {
    if (!m_enabled) return;
    if (!m_maskInfo->isValid()) return;

    KisPaintDeviceSP mask = m_maskInfo->mask();

    const QRect rect = dab->bounds();
    const QRect maskBounds = m_maskInfo->maskBounds();

    KisCachedPaintDevice::Guard g(mask, KoColorSpaceRegistry::instance()->rgb8(), m_cachedPaintDevice);
    KisPaintDeviceSP fillMaskDevice = g.device();

    int x = offset.x() % maskBounds.width() - m_offsetX;
    int y = offset.y() % maskBounds.height() - m_offsetY;

    const QRect maskPatchRect = QRect(x, y, rect.width(), rect.height());

    KisFillPainter fillMaskPainter(fillMaskDevice);
    fillMaskPainter.setCompositeOpId(COMPOSITE_COPY);
    fillMaskPainter.fillRect(kisGrowRect(maskPatchRect, 1), mask, maskBounds);
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

void KisTextureOption::applyGradient(KisFixedPaintDeviceSP dab, const QPoint& offset, const KisPaintInformation& info) {
    if (!m_enabled) return;
    if (!m_maskInfo->isValid()) return;

    KIS_SAFE_ASSERT_RECOVER_RETURN(m_gradient && m_gradient->valid());

    KisPaintDeviceSP mask = m_maskInfo->mask();
    const QRect maskBounds = m_maskInfo->maskBounds();
    QRect rect = dab->bounds();

    KisCachedPaintDevice::Guard g(mask, KoColorSpaceRegistry::instance()->rgb8(), m_cachedPaintDevice);
    KisPaintDeviceSP fillDevice = g.device();

    int x = offset.x() % maskBounds.width() - m_offsetX;
    int y = offset.y() % maskBounds.height() - m_offsetY;

    const QRect maskPatchRect = QRect(x, y, rect.width(), rect.height());

    KisFillPainter fillPainter(fillDevice);
    fillPainter.setCompositeOpId(COMPOSITE_COPY);
    fillPainter.fillRect(kisGrowRect(maskPatchRect, 1), mask, maskBounds);
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

void KisTextureOption::apply(KisFixedPaintDeviceSP dab, const QPoint &offset, const KisPaintInformation & info)
{
    if (!m_enabled) return;
    if (!m_maskInfo->isValid()) return;

    if (m_texturingMode == KisTextureOptionData::LIGHTNESS) {
        applyLightness(dab, offset, info);
        return;
    }
    else if (m_texturingMode == KisTextureOptionData::GRADIENT && m_gradient) {
        applyGradient(dab, offset, info);
        return;
    }

    QRect rect = dab->bounds();
    KisPaintDeviceSP mask = m_maskInfo->mask();
    const QRect maskBounds = m_maskInfo->maskBounds();

    KisCachedPaintDevice::Guard g(mask, KoColorSpaceRegistry::instance()->alpha8(), m_cachedPaintDevice);
    KisPaintDeviceSP maskPatch = g.device();

    int x = offset.x() % maskBounds.width() - m_offsetX;
    int y = offset.y() % maskBounds.height() - m_offsetY;

    const QRect maskPatchRect = QRect(x, y, rect.width(), rect.height());

    KisFillPainter fillPainter(maskPatch);
    fillPainter.setCompositeOpId(COMPOSITE_COPY);
    fillPainter.fillRect(kisGrowRect(maskPatchRect, 1), mask, maskBounds);
    fillPainter.end();

    // Compute final strength
    qreal strength = m_strengthOption.apply(info);

    // Select mask compositing op
    KoChannelInfo::enumChannelValueType alphaChannelType = KoChannelInfo::UINT8;
    int alphaChannelOffset = -1;

    const QList<KoChannelInfo *> channels = dab->colorSpace()->channels();
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
    case KisTextureOptionData::MULTIPLY:
        compositeOp.reset(KisMaskingBrushCompositeOpFactory::createForAlphaSrc(COMPOSITE_MULT, alphaChannelType, dab->pixelSize(), alphaChannelOffset, strength));
        break;
    case KisTextureOptionData::SUBTRACT:
        compositeOp.reset(KisMaskingBrushCompositeOpFactory::createForAlphaSrc(COMPOSITE_SUBTRACT, alphaChannelType, dab->pixelSize(), alphaChannelOffset, strength));
        break;
    case KisTextureOptionData::DARKEN:
        compositeOp.reset(KisMaskingBrushCompositeOpFactory::createForAlphaSrc(COMPOSITE_DARKEN, alphaChannelType, dab->pixelSize(), alphaChannelOffset, strength));
        break;
    case KisTextureOptionData::OVERLAY:
        compositeOp.reset(KisMaskingBrushCompositeOpFactory::createForAlphaSrc(COMPOSITE_OVERLAY, alphaChannelType, dab->pixelSize(), alphaChannelOffset, strength));
        break;
    case KisTextureOptionData::COLOR_DODGE:
        compositeOp.reset(KisMaskingBrushCompositeOpFactory::createForAlphaSrc(COMPOSITE_DODGE, alphaChannelType, dab->pixelSize(), alphaChannelOffset, strength));
        break;
    case KisTextureOptionData::COLOR_BURN:
        compositeOp.reset(KisMaskingBrushCompositeOpFactory::createForAlphaSrc(COMPOSITE_BURN, alphaChannelType, dab->pixelSize(), alphaChannelOffset, strength));
        break;
    case KisTextureOptionData::LINEAR_DODGE:
        compositeOp.reset(KisMaskingBrushCompositeOpFactory::createForAlphaSrc(COMPOSITE_LINEAR_DODGE, alphaChannelType, dab->pixelSize(), alphaChannelOffset, strength));
        break;
    case KisTextureOptionData::LINEAR_BURN:
        compositeOp.reset(KisMaskingBrushCompositeOpFactory::createForAlphaSrc(COMPOSITE_LINEAR_BURN, alphaChannelType, dab->pixelSize(), alphaChannelOffset, strength));
        break;
    case KisTextureOptionData::HARD_MIX_PHOTOSHOP:
        compositeOp.reset(KisMaskingBrushCompositeOpFactory::createForAlphaSrc(COMPOSITE_HARD_MIX_PHOTOSHOP, alphaChannelType, dab->pixelSize(), alphaChannelOffset, strength));
        break;
    case KisTextureOptionData::HARD_MIX_SOFTER_PHOTOSHOP:
        compositeOp.reset(KisMaskingBrushCompositeOpFactory::createForAlphaSrc(COMPOSITE_HARD_MIX_SOFTER_PHOTOSHOP, alphaChannelType, dab->pixelSize(), alphaChannelOffset, strength));
        break;
    case KisTextureOptionData::HEIGHT:
        compositeOp.reset(KisMaskingBrushCompositeOpFactory::createForAlphaSrc("height", alphaChannelType, dab->pixelSize(), alphaChannelOffset, strength));
        break;
    case KisTextureOptionData::LINEAR_HEIGHT:
        compositeOp.reset(KisMaskingBrushCompositeOpFactory::createForAlphaSrc("linear_height", alphaChannelType, dab->pixelSize(), alphaChannelOffset, strength));
        break;
    case KisTextureOptionData::HEIGHT_PHOTOSHOP:
        compositeOp.reset(KisMaskingBrushCompositeOpFactory::createForAlphaSrc("height_photoshop", alphaChannelType, dab->pixelSize(), alphaChannelOffset, strength));
        break;
    case KisTextureOptionData::LINEAR_HEIGHT_PHOTOSHOP:
        compositeOp.reset(KisMaskingBrushCompositeOpFactory::createForAlphaSrc("linear_height_photoshop", alphaChannelType, dab->pixelSize(), alphaChannelOffset, strength));
        break;
    default: return;
    }

    // Apply the mask to the dab
    {
        quint8 *dabIt = nullptr;
        KisRandomConstAccessorSP maskPatchIt = maskPatch->createRandomConstAccessorNG();

        qint32 dabY = dab->bounds().y();
        qint32 maskPatchY = maskPatchRect.y();
        qint32 rowsRemaining = dab->bounds().height();
        const qint32 dabRowStride = dab->bounds().width() * dab->pixelSize();

        while (rowsRemaining > 0) {
            qint32 dabX = dab->bounds().x();
            qint32 maskPatchX = maskPatchRect.x();
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
