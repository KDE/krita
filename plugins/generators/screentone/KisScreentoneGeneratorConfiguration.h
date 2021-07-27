/*
 * KDE. Krita Project.
 *
 * SPDX-FileCopyrightText: 2021 Deif Lou <ginoba@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISSCREENTONEGENERATORCONFIGURATION_H
#define KISSCREENTONEGENERATORCONFIGURATION_H

#include <QString>
#include <QStringList>

#include <KoColor.h>
#include <KoColorSpaceRegistry.h>
#include <kis_filter_configuration.h>

enum KisScreentonePatternType
{
    KisScreentonePatternType_Dots,
    KisScreentonePatternType_Lines
};

enum KisScreentoneShapeType
{
    // Dots
    KisScreentoneShapeType_RoundDots,
    KisScreentoneShapeType_EllipseDotsLegacy,
    KisScreentoneShapeType_DiamondDots,
    KisScreentoneShapeType_SquareDots,
    KisScreentoneShapeType_EllipseDots,
    // Lines
    KisScreentoneShapeType_StraightLines = 0,
    KisScreentoneShapeType_SineWaveLines,
    KisScreentoneShapeType_TriangularWaveLines,
    KisScreentoneShapeType_SawtoothWaveLines,
    KisScreentoneShapeType_CurtainsLines
};

enum KisScreentoneInterpolationType
{
    KisScreentoneInterpolationType_Linear,
    KisScreentoneInterpolationType_Sinusoidal
};

enum KisScreentoneEqualizationMode
{
    KisScreentoneEqualizationMode_None,
    KisScreentoneEqualizationMode_FunctionBased,
    KisScreentoneEqualizationMode_TemplateBased
};

enum KisScreentoneTransformationMode
{
    KisScreentoneTransformationMode_Simple,
    KisScreentoneTransformationMode_Advanced
};

enum KisScreentoneUnits
{
    KisScreentoneUnits_Inches,
    KisScreentoneUnits_Centimeters
};

QStringList screentonePatternNames();
QStringList screentoneShapeNames(int pattern);
QStringList screentoneInterpolationNames(int pattern, int shape);

class KisScreentoneGeneratorConfiguration;
typedef KisPinnedSharedPtr<KisScreentoneGeneratorConfiguration> KisScreentoneGeneratorConfigurationSP;

class KisScreentoneGeneratorConfiguration : public KisFilterConfiguration
{
public:
    KisScreentoneGeneratorConfiguration(qint32 version, KisResourcesInterfaceSP resourcesInterface);
    KisScreentoneGeneratorConfiguration(KisResourcesInterfaceSP resourcesInterface);
    KisScreentoneGeneratorConfiguration(const KisScreentoneGeneratorConfiguration &rhs);

    virtual KisFilterConfigurationSP clone() const override;

    static inline QString defaultName() { return "screentone"; }
    static constexpr qint32 defaultVersion() { return 2; }

    static constexpr int defaultPattern() { return KisScreentonePatternType_Dots; }
    static constexpr int defaultShape() { return KisScreentoneShapeType_RoundDots; }
    static constexpr int defaultInterpolation() { return KisScreentoneInterpolationType_Linear; }
    static constexpr int defaultEqualizationMode() { return KisScreentoneEqualizationMode_FunctionBased; }

    static inline const KoColor& defaultForegroundColor()
    {
        static const KoColor c(Qt::black, KoColorSpaceRegistry::instance()->rgb8());
        return c;
    }
    static inline const KoColor& defaultBackgroundColor()
    {
        static const KoColor c(Qt::white, KoColorSpaceRegistry::instance()->rgb8());
        return c;
    }
    static constexpr int defaultForegroundOpacity() { return 100; }
    static constexpr int defaultBackgroundOpacity() { return 100; }
    static constexpr bool defaultInvert() { return false; }
    static constexpr qreal defaultBrightness() { return 50.0; }
    static constexpr qreal defaultContrast() { return 95.0; }

    static constexpr int defaultTransformationMode() { return KisScreentoneTransformationMode_Simple; }
    static constexpr int defaultUnits() { return KisScreentoneUnits_Inches; }
    static constexpr qreal defaultResolution() { return 300.0; }
    static constexpr qreal defaultFrequencyX() { return 30.0; }
    static constexpr qreal defaultFrequencyY() { return 30.0; }
    static constexpr bool defaultConstrainFrequency() { return true; }
    static constexpr qreal defaultPositionX() { return 0.0; }
    static constexpr qreal defaultPositionY() { return 0.0; }
    static constexpr qreal defaultSizeX() { return 10.0; }
    static constexpr qreal defaultSizeY() { return 10.0; }
    static constexpr bool defaultConstrainSize() { return true; }
    static constexpr qreal defaultShearX() { return 0.0; }
    static constexpr qreal defaultShearY() { return 0.0; }
    static constexpr qreal defaultRotation() { return 45.0; }
    static constexpr bool defaultAlignToPixelGrid() { return false; }
    static constexpr int defaultAlignToPixelGridX() { return 1; }
    static constexpr int defaultAlignToPixelGridY() { return 1; }

    int pattern() const;
    int shape() const;
    int interpolation() const;
    int equalizationMode() const;
    
    KoColor foregroundColor() const;
    KoColor backgroundColor() const;
    int foregroundOpacity() const;
    int backgroundOpacity() const;
    bool invert() const;
    qreal brightness() const;
    qreal contrast() const;

    int transformationMode() const;
    int units() const;
    qreal resolution() const;
    qreal frequencyX() const;
    qreal frequencyY() const;
    bool constrainFrequency() const;
    qreal positionX() const;
    qreal positionY() const;
    qreal sizeX() const;
    qreal sizeY() const;
    bool constrainSize() const;
    qreal shearX() const;
    qreal shearY() const;
    qreal rotation() const;
    bool alignToPixelGrid() const;
    int alignToPixelGridX() const;
    int alignToPixelGridY() const;

    void setPattern(int newPattern);
    void setShape(int newShape);
    void setInterpolation(int newInterpolation);
    void setEqualizationMode(int newEqualizationMode);
    
    void setForegroundColor(const KoColor &newForegroundColor);
    void setBackgroundColor(const KoColor &newBackgroundColor);
    void setForegroundOpacity(int newForegroundOpacity);
    void setBackgroundOpacity(int newBackgroundOpacity);
    void setInvert(bool newInvert);
    void setBrightness(qreal newBrightness);
    void setContrast(qreal newContrast);

    void setTransformationMode(int newTransformationMode);
    void setUnits(int newUnits);
    void setResolution(qreal newResolution);
    void setFrequencyX(qreal newFrequencyX);
    void setFrequencyY(qreal newFrequencyY);
    void setConstrainFrequency(bool newConstrainFrequency);
    void setPositionX(qreal newPositionX);
    void setPositionY(qreal newPositionY);
    void setSizeX(qreal newSizeX);
    void setSizeY(qreal newSizeY);
    void setConstrainSize(bool newConstrainSize);
    void setShearX(qreal newShearX);
    void setShearY(qreal newShearY);
    void setRotation(qreal newRotation);
    void setAlignToPixelGrid(bool newAlignToPixelGrid);
    void setAlignToPixelGridX(int newAlignToPixelGridX);
    void setAlignToPixelGridY(int newAlignToPixelGridY);

    void setDefaults();
};

#endif
