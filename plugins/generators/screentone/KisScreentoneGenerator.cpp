/*
 * KDE. Krita Project.
 *
 * SPDX-FileCopyrightText: 2020 Deif Lou <ginoba@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <utility>

#include <kpluginfactory.h>
#include <KoUpdater.h>
#include <kis_processing_information.h>
#include <KisSequentialIteratorProgress.h>
#include <filter/kis_filter_configuration.h>
#include <generator/kis_generator_registry.h>
#include <KoCompositeOpRegistry.h>
#include <kis_selection.h>
#include <kis_painter.h>
#include <KoColorSpaceRegistry.h>
#include <KoColorProfile.h>

#include "KisScreentoneGenerator.h"
#include "KisScreentoneConfigWidget.h"
#include "KisScreentoneBrightnessContrastFunctions.h"
#include "KisScreentoneScreentoneFunctions.h"

KisScreentoneGenerator::KisScreentoneGenerator() : KisGenerator(id(), KoID("basic"), i18n("&Screentone..."))
{
    setSupportsPainting(true);
}

void KisScreentoneGenerator::generate(KisProcessingInformation dst,
                                      const QSize &size,
                                      const KisFilterConfigurationSP config,
                                      KoUpdater *progressUpdater) const
{
    KIS_SAFE_ASSERT_RECOVER_RETURN(config);

    const KisScreentoneGeneratorConfigurationSP generatorConfiguration =
        dynamic_cast<KisScreentoneGeneratorConfiguration*>(
            const_cast<KisFilterConfiguration*>(config.data())
        );

    return generate(dst, size, generatorConfiguration, progressUpdater);
}

void KisScreentoneGenerator::generate(KisProcessingInformation dst,
                                      const QSize &size,
                                      const KisScreentoneGeneratorConfigurationSP config,
                                      KoUpdater *progressUpdater) const
{
    const int pattern = config->pattern();
    const int shape = config->shape();
    const int interpolation = config->interpolation();

    if (pattern == KisScreentonePatternType_Dots) {
        if (shape == KisScreentoneShapeType_RoundDots) {
            if (interpolation == KisScreentoneInterpolationType_Linear) {
                KisScreentoneScreentoneFunctions::DotsRoundLinear screentoneFunction;
                generate(dst, size, config, progressUpdater, screentoneFunction);
            } else if (interpolation == KisScreentoneInterpolationType_Sinusoidal) {
                KisScreentoneScreentoneFunctions::DotsRoundSinusoidal screentoneFunction;
                generate(dst, size, config, progressUpdater, screentoneFunction);
            }
        } else if (shape == KisScreentoneShapeType_EllipseDotsLegacy) {
            if (interpolation == KisScreentoneInterpolationType_Linear) {
                KisScreentoneScreentoneFunctions::DotsEllipseLinear_Legacy screentoneFunction;
                generate(dst, size, config, progressUpdater, screentoneFunction);
            } else if (interpolation == KisScreentoneInterpolationType_Sinusoidal) {
                KisScreentoneScreentoneFunctions::DotsEllipseSinusoidal_Legacy screentoneFunction;
                generate(dst, size, config, progressUpdater, screentoneFunction);
            }
        } else if (shape == KisScreentoneShapeType_EllipseDots) {
            if (interpolation == KisScreentoneInterpolationType_Linear) {
                KisScreentoneScreentoneFunctions::DotsEllipseLinear screentoneFunction;
                generate(dst, size, config, progressUpdater, screentoneFunction);
            } else if (interpolation == KisScreentoneInterpolationType_Sinusoidal) {
                KisScreentoneScreentoneFunctions::DotsEllipseSinusoidal screentoneFunction;
                generate(dst, size, config, progressUpdater, screentoneFunction);
            }
        } else if (shape == KisScreentoneShapeType_DiamondDots) {
            KisScreentoneScreentoneFunctions::DotsDiamond screentoneFunction;
            generate(dst, size, config, progressUpdater, screentoneFunction);
        } else if (shape == KisScreentoneShapeType_SquareDots) {
            KisScreentoneScreentoneFunctions::DotsSquare screentoneFunction;
            generate(dst, size, config, progressUpdater, screentoneFunction);
        }
    } else if (pattern == KisScreentonePatternType_Lines) {
        if (shape == KisScreentoneShapeType_StraightLines) {
            if (interpolation == KisScreentoneInterpolationType_Linear) {
                KisScreentoneScreentoneFunctions::LinesStraightLinear screentoneFunction;
                generate(dst, size, config, progressUpdater, screentoneFunction);
            } else if (interpolation == KisScreentoneInterpolationType_Sinusoidal) {
                KisScreentoneScreentoneFunctions::LinesStraightSinusoidal screentoneFunction;
                generate(dst, size, config, progressUpdater, screentoneFunction);
            }
        } else if (shape == KisScreentoneShapeType_SineWaveLines) {
            if (interpolation == KisScreentoneInterpolationType_Linear) {
                KisScreentoneScreentoneFunctions::LinesSineWaveLinear screentoneFunction;
                generate(dst, size, config, progressUpdater, screentoneFunction);
            } else if (interpolation == KisScreentoneInterpolationType_Sinusoidal) {
                KisScreentoneScreentoneFunctions::LinesSineWaveSinusoidal screentoneFunction;
                generate(dst, size, config, progressUpdater, screentoneFunction);
            }
        } else if (shape == KisScreentoneShapeType_TriangularWaveLines) {
            if (interpolation == KisScreentoneInterpolationType_Linear) {
                KisScreentoneScreentoneFunctions::LinesTriangularWaveLinear screentoneFunction;
                generate(dst, size, config, progressUpdater, screentoneFunction);
            } else if (interpolation == KisScreentoneInterpolationType_Sinusoidal) {
                KisScreentoneScreentoneFunctions::LinesTriangularWaveSinusoidal screentoneFunction;
                generate(dst, size, config, progressUpdater, screentoneFunction);
            }
        } else if (shape == KisScreentoneShapeType_SawtoothWaveLines) {
            if (interpolation == KisScreentoneInterpolationType_Linear) {
                KisScreentoneScreentoneFunctions::LinesSawToothWaveLinear screentoneFunction;
                generate(dst, size, config, progressUpdater, screentoneFunction);
            } else if (interpolation == KisScreentoneInterpolationType_Sinusoidal) {
                KisScreentoneScreentoneFunctions::LinesSawToothWaveSinusoidal screentoneFunction;
                generate(dst, size, config, progressUpdater, screentoneFunction);
            }
        } else if (shape == KisScreentoneShapeType_CurtainsLines) {
            if (interpolation == KisScreentoneInterpolationType_Linear) {
                KisScreentoneScreentoneFunctions::LinesCurtainsLinear screentoneFunction;
                generate(dst, size, config, progressUpdater, screentoneFunction);
            } else if (interpolation == KisScreentoneInterpolationType_Sinusoidal) {
                KisScreentoneScreentoneFunctions::LinesCurtainsSinusoidal screentoneFunction;
                generate(dst, size, config, progressUpdater, screentoneFunction);
            }
        }
    }
}

template <class ScreentoneFunction>
void KisScreentoneGenerator::generate(KisProcessingInformation dst,
                                      const QSize &size,
                                      const KisScreentoneGeneratorConfigurationSP config,
                                      KoUpdater *progressUpdater,
                                      const ScreentoneFunction &screentoneFunction) const
{
    const qreal contrast = config->contrast() / 50.0 - 1.0;
    const bool useThresholdFunction = qFuzzyCompare(contrast, 1.0);

    if (useThresholdFunction) {
        const qreal brightness = config->brightness() / 100.0;
        KisScreentoneBrightnessContrastFunctions::Threshold thresholdFunction(1.0 - brightness);
        generate(dst, size, config, progressUpdater, screentoneFunction, thresholdFunction);
    } else {
        const qreal brightness = config->brightness() / 50.0 - 1.0;
        const bool bypassBrightnessContrast = qFuzzyIsNull(brightness) && qFuzzyIsNull(contrast);
        if (bypassBrightnessContrast) {
            KisScreentoneBrightnessContrastFunctions::Identity brightnessContrastFunction;
            generate(dst, size, config, progressUpdater, screentoneFunction, brightnessContrastFunction);
        } else {
            KisScreentoneBrightnessContrastFunctions::BrightnessContrast brightnessContrastFunction(brightness, contrast);
            generate(dst, size, config, progressUpdater, screentoneFunction, brightnessContrastFunction);
        }
    }
}

bool KisScreentoneGenerator::checkUpdaterInterruptedAndSetPercent(KoUpdater *progressUpdater, int percent) const
{
    // The updater is null so return false to keep going
    // with the computations
    if (!progressUpdater) {
        return false;
    }

    if (progressUpdater->interrupted()) {
        return true;
    }

    progressUpdater->setProgress(percent);
    return false;
}

template <class ScreentoneFunction, class BrightnessContrastFunction>
void KisScreentoneGenerator::generate(KisProcessingInformation dst,
                                      const QSize &size,
                                      const KisScreentoneGeneratorConfigurationSP config,
                                      KoUpdater *progressUpdater,
                                      const ScreentoneFunction &screentoneFunction,
                                      const BrightnessContrastFunction &brightnessContrastFunction) const
{
    KisPaintDeviceSP device = dst.paintDevice();
    Q_ASSERT(!device.isNull());
    KIS_SAFE_ASSERT_RECOVER_RETURN(config);

    checkUpdaterInterruptedAndSetPercent(progressUpdater, 0);
    
    const QRect bounds = QRect(dst.topLeft(), size);
    const KoColorSpace *colorSpace;
    if (device->colorSpace()->profile()->isLinear()) {
        colorSpace = KoColorSpaceRegistry::instance()->rgb8();
    } else {
        colorSpace = device->colorSpace();
    }
    
    // Get transformation parameters
    qreal positionX, positionY, sizeX, sizeY, shearX, shearY;
    if (config->transformationMode() == KisScreentoneTransformationMode_Advanced) {
        positionX = config->positionX();
        positionY = config->positionY();
        const bool constrainSize = config->constrainSize();
        sizeX = config->sizeX();
        // Ensure that the size y component is equal to the x component if keepSizeSquare is true
        sizeY = constrainSize ? sizeX : config->sizeY();
        shearX = config->shearX();
        shearY = config->shearY();
    } else {
        const qreal resolution = config->resolution();
        const bool constrainFrequency = config->constrainFrequency();
        const qreal frequencyX = config->frequencyX();
        // Ensure that the frequency y component is equal to the x component if constrainFrequency is true
        const qreal frequencyY = constrainFrequency ? frequencyX : config->frequencyY();
        positionX = positionY = 0.0;
        sizeX = resolution / frequencyX;
        sizeY = resolution / frequencyY;
        shearX = shearY = 0.0;
    }
    const qreal rotation = config->rotation();
    
    // Get final transformation
    QTransform t;
    if (config->alignToPixelGrid()) {
        t.rotate(-rotation);
        t.scale(sizeX, sizeY);
        t.shear(-shearX, -shearY);
        const qreal alignX = static_cast<qreal>(config->alignToPixelGridX());
        const qreal alignY = static_cast<qreal>(config->alignToPixelGridY());
        const QPointF u1 = t.map(QPointF(alignX, 0.0));
        const QPointF u2 = t.map(QPointF(0.0, alignY));
        const QPointF v1(qRound(u1.x()), qRound(u1.y()));
        const QPointF v2(qRound(u2.x()), qRound(u2.y()));
        QPolygonF quad;
        quad.append(QPointF(0, 0));
        quad.append(v1 / alignX);
        quad.append(v1 / alignX + v2 / alignY);
        quad.append(v2 / alignY);
        QTransform::quadToSquare(quad, t);
        t.translate(positionX, positionY);
    } else {
        t.shear(shearX, shearY);
        t.scale(qIsNull(sizeX) ? 0.0 : 1.0 / sizeX, qIsNull(sizeY) ? 0.0 : 1.0 / sizeY);
        t.rotate(rotation);
        t.translate(positionX, positionY);
    }

    KoColor foregroundColor = config->foregroundColor();
    KoColor backgroundColor = config->backgroundColor();
    qreal foregroundOpacity = config->foregroundOpacity() / 100.0;
    qreal backgroundOpacity = config->backgroundOpacity() / 100.0;
    foregroundColor.convertTo(colorSpace);
    backgroundColor.convertTo(colorSpace);
    foregroundColor.setOpacity(foregroundOpacity);
    backgroundColor.setOpacity(backgroundOpacity);

    KisPaintDeviceSP foregroundDevice = new KisPaintDevice(colorSpace, "screentone_generator_foreground_paint_device");
    KisPaintDeviceSP backgroundDevice;
    if (device->colorSpace()->profile()->isLinear()) {
        backgroundDevice = new KisPaintDevice(colorSpace, "screentone_generator_background_paint_device");
    } else {
        backgroundDevice = device;
    }

    foregroundDevice->fill(bounds, foregroundColor);
    checkUpdaterInterruptedAndSetPercent(progressUpdater, 25);
    backgroundDevice->fill(bounds, backgroundColor);
    checkUpdaterInterruptedAndSetPercent(progressUpdater, 50);

    KisSelectionSP selection = new KisSelection(device->defaultBounds());
    KisSequentialIterator it(selection->pixelSelection(), bounds);

    if (!config->invert()) {
        while (it.nextPixel()) {
            qreal x, y;
            t.map(it.x() + 0.5, it.y() + 0.5, &x, &y);

            qreal v = KisScreentoneScreentoneFunctions::roundValue(screentoneFunction(x, y));
            v = qBound(0.0, brightnessContrastFunction(v), 1.0);
            *it.rawData() = 255 - static_cast<quint8>(qRound(v * 255.0));
        }
    } else {
        while (it.nextPixel()) {
            qreal x, y;
            t.map(it.x() + 0.5, it.y() + 0.5, &x, &y);

            qreal v = KisScreentoneScreentoneFunctions::roundValue(screentoneFunction(x, y));
            v = qBound(0.0, brightnessContrastFunction(v), 1.0);
            *it.rawData() = static_cast<quint8>(qRound(v * 255.0));
        }
    }
    checkUpdaterInterruptedAndSetPercent(progressUpdater, 25);

    {
        KisPainter gc(backgroundDevice, selection);
        gc.setCompositeOp(COMPOSITE_OVER);
        gc.bitBlt(bounds.topLeft(), foregroundDevice, bounds);
    }
    if (device->colorSpace()->profile()->isLinear()) {
        KisPainter gc(device);
        gc.setCompositeOp(COMPOSITE_COPY);
        gc.bitBlt(bounds.topLeft(), backgroundDevice, bounds);
    }
    checkUpdaterInterruptedAndSetPercent(progressUpdater, 100);
}

KisFilterConfigurationSP KisScreentoneGenerator::factoryConfiguration(KisResourcesInterfaceSP resourcesInterface) const
{
    return new KisScreentoneGeneratorConfiguration(resourcesInterface);
}

KisFilterConfigurationSP KisScreentoneGenerator::defaultConfiguration(KisResourcesInterfaceSP resourcesInterface) const
{
    KisScreentoneGeneratorConfigurationSP config = 
        dynamic_cast<KisScreentoneGeneratorConfiguration*>(factoryConfiguration(resourcesInterface).data());
    config->setDefaults();
    return config;
}

KisConfigWidget * KisScreentoneGenerator::createConfigurationWidget(QWidget* parent, const KisPaintDeviceSP dev, bool) const
{
    Q_UNUSED(dev);
    return new KisScreentoneConfigWidget(parent);
}
