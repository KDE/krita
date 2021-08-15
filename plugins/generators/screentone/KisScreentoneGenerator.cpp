/*
 * KDE. Krita Project.
 *
 * SPDX-FileCopyrightText: 2020 Deif Lou <ginoba@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

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
#include "KisScreentoneGeneratorFunctionSampler.h"
#include "KisScreentoneGeneratorTemplate.h"
#include "KisScreentoneGeneratorTemplateSampler.h"

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
    const int equalizationMode = config->equalizationMode();

    if (equalizationMode == KisScreentoneEqualizationMode_TemplateBased) {
        const KisScreentoneGeneratorTemplate &t = config->getTemplate();
        if (config->alignToPixelGrid()) {
            KisScreentoneGeneratorAlignedTemplateSampler<KisScreentoneGeneratorTemplate> sampler(t);
            generate(dst, size, config, progressUpdater, sampler);
        } else {
            KisScreentoneGeneratorUnAlignedTemplateSampler<KisScreentoneGeneratorTemplate> sampler(t);
            generate(dst, size, config, progressUpdater, sampler);
        }
        return;
    }

    const int pattern = config->pattern();
    const int shape = config->shape();
    const int interpolation = config->interpolation();

    {
        using namespace KisScreentoneScreentoneFunctions;

        if (pattern == KisScreentonePatternType_Dots) {
            if (shape == KisScreentoneShapeType_RoundDots) {
                if (interpolation == KisScreentoneInterpolationType_Linear) {
                    if (equalizationMode == KisScreentoneEqualizationMode_FunctionBased) {
                        KisScreentoneGeneratorFunctionSampler<DotsRoundLinearEqualized> sampler(config);
                        generate(dst, size, config, progressUpdater, sampler);
                    } else {
                        KisScreentoneGeneratorFunctionSampler<DotsRoundLinear> sampler(config);
                        generate(dst, size, config, progressUpdater, sampler);
                    }
                } else if (interpolation == KisScreentoneInterpolationType_Sinusoidal) {
                    if (equalizationMode == KisScreentoneEqualizationMode_FunctionBased) {
                        KisScreentoneGeneratorFunctionSampler<DotsRoundSinusoidalEqualized> sampler(config);
                        generate(dst, size, config, progressUpdater, sampler);
                    } else {
                        KisScreentoneGeneratorFunctionSampler<DotsRoundSinusoidal> sampler(config);
                        generate(dst, size, config, progressUpdater, sampler);
                    }
                }
            } else if (shape == KisScreentoneShapeType_EllipseDotsLegacy) {
                if (interpolation == KisScreentoneInterpolationType_Linear) {
                    if (equalizationMode == KisScreentoneEqualizationMode_FunctionBased) {
                        KisScreentoneGeneratorFunctionSampler<DotsEllipseLinearEqualized_Legacy> sampler(config);
                        generate(dst, size, config, progressUpdater, sampler);
                    } else {
                        KisScreentoneGeneratorFunctionSampler<DotsEllipseLinear_Legacy> sampler(config);
                        generate(dst, size, config, progressUpdater, sampler);
                    }
                } else if (interpolation == KisScreentoneInterpolationType_Sinusoidal) {
                    if (equalizationMode == KisScreentoneEqualizationMode_FunctionBased) {
                        KisScreentoneGeneratorFunctionSampler<DotsEllipseSinusoidalEqualized_Legacy> sampler(config);
                        generate(dst, size, config, progressUpdater, sampler);
                    } else {
                        KisScreentoneGeneratorFunctionSampler<DotsEllipseSinusoidal_Legacy> sampler(config);
                        generate(dst, size, config, progressUpdater, sampler);
                    }
                }
            } else if (shape == KisScreentoneShapeType_EllipseDots) {
                if (interpolation == KisScreentoneInterpolationType_Linear) {
                    if (equalizationMode == KisScreentoneEqualizationMode_FunctionBased) {
                        KisScreentoneGeneratorFunctionSampler<DotsEllipseLinearEqualized> sampler(config);
                        generate(dst, size, config, progressUpdater, sampler);
                    } else {
                        KisScreentoneGeneratorFunctionSampler<DotsEllipseLinear> sampler(config);
                        generate(dst, size, config, progressUpdater, sampler);
                    }
                } else if (interpolation == KisScreentoneInterpolationType_Sinusoidal) {
                    if (equalizationMode == KisScreentoneEqualizationMode_FunctionBased) {
                        KisScreentoneGeneratorFunctionSampler<DotsEllipseSinusoidalEqualized> sampler(config);
                        generate(dst, size, config, progressUpdater, sampler);
                    } else {
                        KisScreentoneGeneratorFunctionSampler<DotsEllipseSinusoidal> sampler(config);
                        generate(dst, size, config, progressUpdater, sampler);
                    }
                }
            } else if (shape == KisScreentoneShapeType_DiamondDots) {
                if (equalizationMode == KisScreentoneEqualizationMode_FunctionBased) {
                    KisScreentoneGeneratorFunctionSampler<DotsDiamondEqualized> sampler(config);
                    generate(dst, size, config, progressUpdater, sampler);
                } else {
                    KisScreentoneGeneratorFunctionSampler<DotsDiamond> sampler(config);
                    generate(dst, size, config, progressUpdater, sampler);
                }
            } else if (shape == KisScreentoneShapeType_SquareDots) {
                if (equalizationMode == KisScreentoneEqualizationMode_FunctionBased) {
                    KisScreentoneGeneratorFunctionSampler<DotsSquareEqualized> sampler(config);
                    generate(dst, size, config, progressUpdater, sampler);
                } else {
                    KisScreentoneGeneratorFunctionSampler<DotsSquare> sampler(config);
                    generate(dst, size, config, progressUpdater, sampler);
                }
            }
        } else if (pattern == KisScreentonePatternType_Lines) {
            if (shape == KisScreentoneShapeType_StraightLines) {
                if (interpolation == KisScreentoneInterpolationType_Linear) {
                    if (equalizationMode == KisScreentoneEqualizationMode_FunctionBased) {
                        KisScreentoneGeneratorFunctionSampler<LinesStraightLinearEqualized> sampler(config);
                        generate(dst, size, config, progressUpdater, sampler);
                    } else {
                        KisScreentoneGeneratorFunctionSampler<LinesStraightLinear> sampler(config);
                        generate(dst, size, config, progressUpdater, sampler);
                    }
                } else if (interpolation == KisScreentoneInterpolationType_Sinusoidal) {
                    if (equalizationMode == KisScreentoneEqualizationMode_FunctionBased) {
                        KisScreentoneGeneratorFunctionSampler<LinesStraightSinusoidalEqualized> sampler(config);
                        generate(dst, size, config, progressUpdater, sampler);
                    } else {
                        KisScreentoneGeneratorFunctionSampler<LinesStraightSinusoidal> sampler(config);
                        generate(dst, size, config, progressUpdater, sampler);
                    }
                }
            } else if (shape == KisScreentoneShapeType_SineWaveLines) {
                if (interpolation == KisScreentoneInterpolationType_Linear) {
                    if (equalizationMode == KisScreentoneEqualizationMode_FunctionBased) {
                        KisScreentoneGeneratorFunctionSampler<LinesSineWaveLinearEqualized> sampler(config);
                        generate(dst, size, config, progressUpdater, sampler);
                    } else {
                        KisScreentoneGeneratorFunctionSampler<LinesSineWaveLinear> sampler(config);
                        generate(dst, size, config, progressUpdater, sampler);
                    }
                } else if (interpolation == KisScreentoneInterpolationType_Sinusoidal) {
                    if (equalizationMode == KisScreentoneEqualizationMode_FunctionBased) {
                        KisScreentoneGeneratorFunctionSampler<LinesSineWaveSinusoidalEqualized> sampler(config);
                        generate(dst, size, config, progressUpdater, sampler);
                    } else {
                        KisScreentoneGeneratorFunctionSampler<LinesSineWaveSinusoidal> sampler(config);
                        generate(dst, size, config, progressUpdater, sampler);
                    }
                }
            } else if (shape == KisScreentoneShapeType_TriangularWaveLines) {
                if (interpolation == KisScreentoneInterpolationType_Linear) {
                    if (equalizationMode == KisScreentoneEqualizationMode_FunctionBased) {
                        KisScreentoneGeneratorFunctionSampler<LinesTriangularWaveLinearEqualized> sampler(config);
                        generate(dst, size, config, progressUpdater, sampler);
                    } else {
                        KisScreentoneGeneratorFunctionSampler<LinesTriangularWaveLinear> sampler(config);
                        generate(dst, size, config, progressUpdater, sampler);
                    }
                } else if (interpolation == KisScreentoneInterpolationType_Sinusoidal) {
                    if (equalizationMode == KisScreentoneEqualizationMode_FunctionBased) {
                        KisScreentoneGeneratorFunctionSampler<LinesTriangularWaveSinusoidalEqualized> sampler(config);
                        generate(dst, size, config, progressUpdater, sampler);
                    } else {
                        KisScreentoneGeneratorFunctionSampler<LinesTriangularWaveSinusoidal> sampler(config);
                        generate(dst, size, config, progressUpdater, sampler);
                    }
                }
            } else if (shape == KisScreentoneShapeType_SawtoothWaveLines) {
                if (interpolation == KisScreentoneInterpolationType_Linear) {
                    if (equalizationMode == KisScreentoneEqualizationMode_FunctionBased) {
                        KisScreentoneGeneratorFunctionSampler<LinesSawToothWaveLinearEqualized> sampler(config);
                        generate(dst, size, config, progressUpdater, sampler);
                    } else {
                        KisScreentoneGeneratorFunctionSampler<LinesSawToothWaveLinear> sampler(config);
                        generate(dst, size, config, progressUpdater, sampler);
                    }
                } else if (interpolation == KisScreentoneInterpolationType_Sinusoidal) {
                    if (equalizationMode == KisScreentoneEqualizationMode_FunctionBased) {
                        KisScreentoneGeneratorFunctionSampler<LinesSawToothWaveSinusoidalEqualized> sampler(config);
                        generate(dst, size, config, progressUpdater, sampler);
                    } else {
                        KisScreentoneGeneratorFunctionSampler<LinesSawToothWaveSinusoidal> sampler(config);
                        generate(dst, size, config, progressUpdater, sampler);
                    }
                }
            } else if (shape == KisScreentoneShapeType_CurtainsLines) {
                if (interpolation == KisScreentoneInterpolationType_Linear) {
                    if (equalizationMode == KisScreentoneEqualizationMode_FunctionBased) {
                        KisScreentoneGeneratorFunctionSampler<LinesCurtainsLinearEqualized> sampler(config);
                        generate(dst, size, config, progressUpdater, sampler);
                    } else {
                        KisScreentoneGeneratorFunctionSampler<LinesCurtainsLinear> sampler(config);
                        generate(dst, size, config, progressUpdater, sampler);
                    }
                } else if (interpolation == KisScreentoneInterpolationType_Sinusoidal) {
                    if (equalizationMode == KisScreentoneEqualizationMode_FunctionBased) {
                        KisScreentoneGeneratorFunctionSampler<LinesCurtainsSinusoidalEqualized> sampler(config);
                        generate(dst, size, config, progressUpdater, sampler);
                    } else {
                        KisScreentoneGeneratorFunctionSampler<LinesCurtainsSinusoidal> sampler(config);
                        generate(dst, size, config, progressUpdater, sampler);
                    }
                }
            }
        }
    }
}

template <class Sampler>
void KisScreentoneGenerator::generate(KisProcessingInformation dst,
                                      const QSize &size,
                                      const KisScreentoneGeneratorConfigurationSP config,
                                      KoUpdater *progressUpdater,
                                      const Sampler &sampler) const
{
    const qreal contrast = config->contrast() / 50.0 - 1.0;
    const bool useThresholdFunction = qFuzzyCompare(contrast, 1.0);

    if (useThresholdFunction) {
        const qreal brightness = config->brightness() / 100.0;
        KisScreentoneBrightnessContrastFunctions::Threshold thresholdFunction(1.0 - brightness);
        generate(dst, size, config, progressUpdater, sampler, thresholdFunction);
    } else {
        const qreal brightness = config->brightness() / 50.0 - 1.0;
        const bool bypassBrightnessContrast = qFuzzyIsNull(brightness) && qFuzzyIsNull(contrast);
        if (bypassBrightnessContrast) {
            KisScreentoneBrightnessContrastFunctions::Identity brightnessContrastFunction;
            generate(dst, size, config, progressUpdater, sampler, brightnessContrastFunction);
        } else {
            KisScreentoneBrightnessContrastFunctions::BrightnessContrast brightnessContrastFunction(brightness, contrast);
            generate(dst, size, config, progressUpdater, sampler, brightnessContrastFunction);
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

template <class Sampler, class PostprocessingFunction>
void KisScreentoneGenerator::generate(KisProcessingInformation dst,
                                      const QSize &size,
                                      const KisScreentoneGeneratorConfigurationSP config,
                                      KoUpdater *progressUpdater,
                                      const Sampler &sampler,
                                      const PostprocessingFunction &postprocessingFunction) const
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
            qreal v = std::round(sampler(it.x(), it.y()) * 10000.0) / 10000.0;
            v = qBound(0.0, postprocessingFunction(v), 1.0);
            *it.rawData() = 255 - static_cast<quint8>(qRound(v * 255.0));
        }
    } else {
        while (it.nextPixel()) {
            qreal v = std::round(sampler(it.x(), it.y()) * 10000.0) / 10000.0;
            v = qBound(0.0, postprocessingFunction(v), 1.0);
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
