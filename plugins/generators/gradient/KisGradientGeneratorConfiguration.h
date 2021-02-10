/*
 * KDE. Krita Project.
 *
 * SPDX-FileCopyrightText: 2020 Deif Lou <ginoba@gmail.com>
 * SPDX-FileCopyrightText: 2021 L. E. Segovia <amy@amyspark.me>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISGRADIENTGENERATORCONFIGURATION_H
#define KISGRADIENTGENERATORCONFIGURATION_H

#include <QString>
#include <QStringList>

#include <kis_filter_configuration.h>
#include <kis_gradient_painter.h>
#include <KoAbstractGradient.h>
#include <KoStopGradient.h>
#include <KoColorSpaceRegistry.h>

class KisGradientGeneratorConfiguration;
typedef KisPinnedSharedPtr<KisGradientGeneratorConfiguration> KisGradientGeneratorConfigurationSP;

class KisGradientGeneratorConfiguration : public KisFilterConfiguration
{
public:
    enum CoordinateSystem
    {
        CoordinateSystemCartesian,
        CoordinateSystemPolar
    };

    enum SpatialUnits
    {
        SpatialUnitsPixels,
        SpatialUnitsPercentOfWidth,
        SpatialUnitsPercentOfHeight,
        SpatialUnitsPercentOfLongestSide,
        SpatialUnitsPercentOfShortestSide
    };

    enum Positioning
    {
        PositioningAbsolute,
        PositioningRelative
    };

    enum GradientType
    {
        GradientTypeStop,
        GradientTypeSegment
    };

    KisGradientGeneratorConfiguration(qint32 version, KisResourcesInterfaceSP resourcesInterface);
    KisGradientGeneratorConfiguration(KisResourcesInterfaceSP resourcesInterface);
    KisGradientGeneratorConfiguration(const KisGradientGeneratorConfiguration &rhs);

    virtual KisFilterConfigurationSP clone() const override;

    static inline QString defaultName()
    {
        return "gradient";
    }
    
    static constexpr qint32 defaultVersion()
    {
        return 1;
    }

    static constexpr KisGradientPainter::enumGradientShape defaultShape()
    {
        return KisGradientPainter::GradientShapeLinear;
    }

    static constexpr KisGradientPainter::enumGradientRepeat defaultRepeat()
    {
        return KisGradientPainter::GradientRepeatNone;
    }

    static constexpr qreal defaultAntiAliasThreshold()
    {
        return 0.0;
    }

    static constexpr bool defaultDither()
    {
        return false;
    }

    static constexpr bool defaultReverse()
    {
        return false;
    }

    static constexpr qreal defaultStartPositionX()
    {
        return 0.0;
    }

    static constexpr qreal defaultStartPositionY()
    {
        return 50.0;
    }
    
    static constexpr SpatialUnits defaultStartPositionXUnits()
    {
        return SpatialUnitsPercentOfWidth;
    }
    
    static constexpr SpatialUnits defaultStartPositionYUnits() 
    {
        return SpatialUnitsPercentOfHeight;
    }
    
    static constexpr CoordinateSystem defaultEndPositionCoordinateSystem()
    {
        return CoordinateSystemCartesian;
    }
    
    static constexpr qreal defaultEndPositionX()
    {
        return 100.0;
    }
    
    static constexpr qreal defaultEndPositionY()
    {
        return 50.0;
    }
    
    static constexpr SpatialUnits defaultEndPositionXUnits()
    {
        return SpatialUnitsPercentOfWidth;
    }
    
    static constexpr SpatialUnits defaultEndPositionYUnits()
    {
        return SpatialUnitsPercentOfHeight;
    }
    
    static constexpr Positioning defaultEndPositionXPositioning()
    {
        return PositioningAbsolute;
    }
    
    static constexpr Positioning defaultEndPositionYPositioning()
    {
        return PositioningAbsolute;
    }
    
    static constexpr qreal defaultEndPositionAngle()
    {
        return 0.0;
    }
    
    static constexpr qreal defaultEndPositionDistance()
    {
        return 100.0;
    }
    
    static constexpr SpatialUnits defaultEndPositionDistanceUnits()
    {
        return SpatialUnitsPercentOfWidth;
    }

    static inline KoAbstractGradientSP defaultGradient()
    {
        KoStopGradientSP gradient = KoStopGradientSP(new KoStopGradient);
        gradient->setStops(
            QList<KoGradientStop>()
            << KoGradientStop(0.0, KoColor(Qt::black, KoColorSpaceRegistry::instance()->rgb8(0)), FOREGROUNDSTOP)
            << KoGradientStop(1.0, KoColor(Qt::white, KoColorSpaceRegistry::instance()->rgb8(0)), BACKGROUNDSTOP)
        );
        gradient->setName(i18nc("Default gradient name for the gradient generator", "Unnamed"));
        gradient->setValid(true);
        return gradient;
    }

    KisGradientPainter::enumGradientShape shape() const;
    KisGradientPainter::enumGradientRepeat repeat() const;
    qreal antiAliasThreshold() const;
    bool dither() const;
    bool reverse() const;
    qreal startPositionX() const;
    qreal startPositionY() const;
    SpatialUnits startPositionXUnits() const;
    SpatialUnits startPositionYUnits() const;
    CoordinateSystem endPositionCoordinateSystem() const;
    qreal endPositionX() const;
    qreal endPositionY() const;
    SpatialUnits endPositionXUnits() const;
    SpatialUnits endPositionYUnits() const;
    Positioning endPositionXPositioning() const;
    Positioning endPositionYPositioning() const;
    qreal endPositionAngle() const;
    qreal endPositionDistance() const;
    SpatialUnits endPositionDistanceUnits() const;
    KoAbstractGradientSP gradient() const;

    QPair<QPointF, QPointF> absoluteCartesianPositionsInPixels(int width, int height) const;

    void setShape(KisGradientPainter::enumGradientShape newShape);
    void setRepeat(KisGradientPainter::enumGradientRepeat newRepeat);
    void setAntiAliasThreshold(qreal newAntiAliasThreshold);
    void setDither(bool newDither);
    void setReverse(bool newReverse);
    void setStartPositionX(qreal newStartPositionX);
    void setStartPositionY(qreal newStartPositionY);
    void setStartPositionXUnits(SpatialUnits newStartPositionXUnits);
    void setStartPositionYUnits(SpatialUnits newStartPositionYUnits);
    void setEndPositionCoordinateSystem(CoordinateSystem newEndPositionCoordinateSystem);
    void setEndPositionX(qreal newEndPositionX);
    void setEndPositionY(qreal newEndPositionY);
    void setEndPositionXUnits(SpatialUnits newEndPositionXUnits);
    void setEndPositionYUnits(SpatialUnits newEndPositionYUnits);
    void setEndPositionXPositioning(Positioning newEndPositionXPositioning);
    void setEndPositionYPositioning(Positioning newEndPositionYPositioning);
    void setEndPositionAngle(qreal newEndPositionAngle);
    void setEndPositionDistance(qreal newEndPositionDistance);
    void setEndPositionDistanceUnits(SpatialUnits newEndPositionDistanceUnits);
    void setGradient(KoAbstractGradientSP newGradient);
    void setDefaults();

    static inline QString shapeToString(KisGradientPainter::enumGradientShape shape,
                                        const QString & defaultShapeString = QString())
    {
        if (shape == KisGradientPainter::GradientShapeLinear) {
            return "linear";
        } else if (shape == KisGradientPainter::GradientShapeBiLinear) {
            return "bilinear";
        } else if (shape == KisGradientPainter::GradientShapeRadial) {
            return "radial";
        } else if (shape == KisGradientPainter::GradientShapeSquare) {
            return "square";
        } else if (shape == KisGradientPainter::GradientShapeConical) {
            return "conical";
        } else if (shape == KisGradientPainter::GradientShapeConicalSymetric) {
            return "conical_symetric";
        } else if (shape == KisGradientPainter::GradientShapeSpiral) {
            return "spiral";
        } else if (shape == KisGradientPainter::GradientShapeReverseSpiral) {
            return "reverse_spiral";
        } else if (shape == KisGradientPainter::GradientShapePolygonal) {
            return "polygonal";
        }
        return defaultShapeString;
    }

    static inline KisGradientPainter::enumGradientShape stringToShape(QString const & shapeString,
                                                                      KisGradientPainter::enumGradientShape defaultShape = KisGradientPainter::GradientShapeLinear)
    {
        if (shapeString == "linear") {
            return KisGradientPainter::GradientShapeLinear;
        } else if (shapeString == "bilinear") {
            return KisGradientPainter::GradientShapeBiLinear;
        } else if (shapeString == "radial") {
            return KisGradientPainter::GradientShapeRadial;
        } else if (shapeString == "square") {
            return KisGradientPainter::GradientShapeSquare;
        } else if (shapeString == "conical") {
            return KisGradientPainter::GradientShapeConical;
        } else if (shapeString == "conical_symetric") {
            return KisGradientPainter::GradientShapeConicalSymetric;
        } else if (shapeString == "spiral") {
            return KisGradientPainter::GradientShapeSpiral;
        } else if (shapeString == "reverse_spiral") {
            return KisGradientPainter::GradientShapeReverseSpiral;
        } else if (shapeString == "polygonal") {
            return KisGradientPainter::GradientShapePolygonal;
        }
        return defaultShape;
    }

    static inline QString repeatToString(KisGradientPainter::enumGradientRepeat repeat,
                                         const QString & defaultRepeatString = QString())
    {
        if (repeat == KisGradientPainter::GradientRepeatNone) {
            return "none";
        } else if (repeat == KisGradientPainter::GradientRepeatForwards) {
            return "forwards";
        } else if (repeat == KisGradientPainter::GradientRepeatAlternate) {
            return "alternate";
        }
        return defaultRepeatString;
    }

    static inline KisGradientPainter::enumGradientRepeat stringToRepeat(QString const & repeatString,
                                                                        KisGradientPainter::enumGradientRepeat defaultRepeat = KisGradientPainter::GradientRepeatNone)
    {
        if (repeatString == "none") {
            return KisGradientPainter::GradientRepeatNone;
        } else if (repeatString == "forwards") {
            return KisGradientPainter::GradientRepeatForwards;
        } else if (repeatString == "alternate") {
            return KisGradientPainter::GradientRepeatAlternate;
        }
        return defaultRepeat;
    }

    static inline QString coordinateSystemToString(CoordinateSystem coordinateSystem,
                                                   const QString &defaultCoordinateSystemString = QString())
    {
        if (coordinateSystem == CoordinateSystemCartesian) {
            return "cartesian";
        } else if (coordinateSystem == CoordinateSystemPolar) {
            return "polar";
        }
        return defaultCoordinateSystemString;
    }

    static inline CoordinateSystem stringToCoordinateSystem(QString const & coordinateSystemString,
                                                            CoordinateSystem defaultCoordinateSystem = CoordinateSystemCartesian)
    {
        if (coordinateSystemString == "cartesian") {
            return CoordinateSystemCartesian;
        } else if (coordinateSystemString == "polar") {
            return CoordinateSystemPolar;
        }
        return defaultCoordinateSystem;
    }

    static inline QString spatialUnitsToString(SpatialUnits spatialUnits,
                                               const QString &defaultSpatialUnitsString = QString())
    {
        if (spatialUnits == SpatialUnitsPixels) {
            return "pixels";
        } else if (spatialUnits == SpatialUnitsPercentOfWidth) {
            return "percent_of_width";
        } else if (spatialUnits == SpatialUnitsPercentOfHeight) {
            return "percent_of_height";
        } else if (spatialUnits == SpatialUnitsPercentOfLongestSide) {
            return "percent_of_longest_side";
        } else if (spatialUnits == SpatialUnitsPercentOfShortestSide) {
            return "percent_of_shortest_side";
        }
        return defaultSpatialUnitsString;
    }

    static inline SpatialUnits stringToSpatialUnits(QString const & spatialUnitsString,
                                                    SpatialUnits defaultSpatialUnits = SpatialUnitsPixels)
    {
        if (spatialUnitsString == "pixels") {
            return SpatialUnitsPixels;
        } else if (spatialUnitsString == "percent_of_width") {
            return SpatialUnitsPercentOfWidth;
        } else if (spatialUnitsString == "percent_of_height") {
            return SpatialUnitsPercentOfHeight;
        } else if (spatialUnitsString == "percent_of_longest_side") {
            return SpatialUnitsPercentOfLongestSide;
        } else if (spatialUnitsString == "percent_of_shortest_side") {
            return SpatialUnitsPercentOfShortestSide;
        }
        return defaultSpatialUnits;
    }

    static inline QString positioningToString(Positioning positioning,
                                              const QString &defaultPositioningString = QString())
    {
        if (positioning == PositioningAbsolute) {
            return "absolute";
        } else if (positioning == PositioningRelative) {
            return "relative";
        }
        return defaultPositioningString;
    }

    static inline Positioning stringToPositioning(QString const & positioningString,
                                                  Positioning defaultPositioning = PositioningAbsolute)
    {
        if (positioningString == "absolute") {
            return PositioningAbsolute;
        } else if (positioningString == "relative") {
            return PositioningRelative;
        }
        return defaultPositioning;
    }

private:
    static inline qreal convertUnitsToPixels(qreal x, SpatialUnits sourceUnits, int width, int height)
    {
        if (sourceUnits == SpatialUnitsPercentOfWidth) {
            return x * static_cast<qreal>(width) / 100.0;
        } else if (sourceUnits == SpatialUnitsPercentOfHeight) {
            return x * static_cast<qreal>(height) / 100.0;
        } else if (sourceUnits == SpatialUnitsPercentOfLongestSide) {
            return x * static_cast<qreal>(qMax(width, height)) / 100.0;
        } else if (sourceUnits == SpatialUnitsPercentOfShortestSide) {
            return x * static_cast<qreal>(qMin(width, height)) / 100.0;
        }
        return x;
    }
};

#endif
