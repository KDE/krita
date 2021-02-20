/*
 * KDE. Krita Project.
 *
 * SPDX-FileCopyrightText: 2020 Deif Lou <ginoba@gmail.com>
 * SPDX-FileCopyrightText: 2021 L. E. Segovia <amy@amyspark.me>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <QDomDocument>

#include <KoSegmentGradient.h>

#include <cmath>

#include "KisGradientGeneratorConfiguration.h"

KisGradientGeneratorConfiguration::KisGradientGeneratorConfiguration(qint32 version, KisResourcesInterfaceSP resourcesInterface)
    : KisFilterConfiguration(defaultName(), version, resourcesInterface)
{}

KisGradientGeneratorConfiguration::KisGradientGeneratorConfiguration(KisResourcesInterfaceSP resourcesInterface)
    : KisFilterConfiguration(defaultName(), defaultVersion(), resourcesInterface)
{}

KisGradientGeneratorConfiguration::KisGradientGeneratorConfiguration(const KisGradientGeneratorConfiguration &rhs)
    : KisFilterConfiguration(rhs)
{}

KisFilterConfigurationSP KisGradientGeneratorConfiguration::clone() const
{
    return new KisGradientGeneratorConfiguration(*this);
}

KisGradientPainter::enumGradientShape KisGradientGeneratorConfiguration::shape() const
{
    return stringToShape(getString("shape"), defaultShape());
}

KisGradientPainter::enumGradientRepeat KisGradientGeneratorConfiguration::repeat() const
{
    return stringToRepeat(getString("repeat"), defaultRepeat());
}

qreal KisGradientGeneratorConfiguration::antiAliasThreshold() const
{
    return getDouble("antialias_threshold", defaultAntiAliasThreshold());
}

bool KisGradientGeneratorConfiguration::reverse() const
{
    return getBool("reverse", defaultReverse());
}

bool KisGradientGeneratorConfiguration::dither() const
{
    return getBool("dither", defaultDither());
}

qreal KisGradientGeneratorConfiguration::startPositionX() const
{
    return getDouble("start_position_x", defaultStartPositionX());
}

qreal KisGradientGeneratorConfiguration::startPositionY() const
{
    return getDouble("start_position_y", defaultStartPositionY());
}

KisGradientGeneratorConfiguration::SpatialUnits KisGradientGeneratorConfiguration::startPositionXUnits() const
{
    return stringToSpatialUnits(getString("start_position_x_units"), defaultStartPositionXUnits());
}

KisGradientGeneratorConfiguration::SpatialUnits KisGradientGeneratorConfiguration::startPositionYUnits() const
{
    return stringToSpatialUnits(getString("start_position_y_units"), defaultStartPositionYUnits());
}

KisGradientGeneratorConfiguration::CoordinateSystem KisGradientGeneratorConfiguration::endPositionCoordinateSystem() const
{
    return stringToCoordinateSystem(getString("end_positiom_coordinate_system"), defaultEndPositionCoordinateSystem());
}

qreal KisGradientGeneratorConfiguration::endPositionX() const
{
    return getDouble("end_position_x", defaultEndPositionX());
}

qreal KisGradientGeneratorConfiguration::endPositionY() const
{
    return getDouble("end_position_y", defaultEndPositionY());
}

KisGradientGeneratorConfiguration::SpatialUnits KisGradientGeneratorConfiguration::endPositionXUnits() const
{
    return stringToSpatialUnits(getString("end_position_x_units"), defaultEndPositionXUnits());
}

KisGradientGeneratorConfiguration::SpatialUnits KisGradientGeneratorConfiguration::endPositionYUnits() const
{
    return stringToSpatialUnits(getString("end_position_y_units"), defaultEndPositionYUnits());
}

KisGradientGeneratorConfiguration::Positioning KisGradientGeneratorConfiguration::endPositionXPositioning() const
{
    return stringToPositioning(getString("end_position_x_positioning"), defaultEndPositionXPositioning());
}

KisGradientGeneratorConfiguration::Positioning KisGradientGeneratorConfiguration::endPositionYPositioning() const
{
    return stringToPositioning(getString("end_position_y_positioning"), defaultEndPositionYPositioning());
}

qreal KisGradientGeneratorConfiguration::endPositionAngle() const
{
    return getDouble("end_position_angle", defaultEndPositionAngle());
}
qreal KisGradientGeneratorConfiguration::endPositionDistance() const
{
    return getDouble("end_position_distance", defaultEndPositionDistance());
}

KisGradientGeneratorConfiguration::SpatialUnits KisGradientGeneratorConfiguration::endPositionDistanceUnits() const
{
    return stringToSpatialUnits(getString("end_position_distance_units"), defaultEndPositionDistanceUnits());
}

KoAbstractGradientSP KisGradientGeneratorConfiguration::gradient() const
{
    QDomDocument document;
    if (document.setContent(this->getString("gradient", ""))) {
        const QDomElement gradientElement = document.firstChildElement();
        if (!gradientElement.isNull()) {
            const QString gradientType = gradientElement.attribute("type");
            KoAbstractGradientSP gradient;
            if (gradientType == "stop") {
                gradient = KoStopGradient::fromXML(gradientElement).clone().dynamicCast<KoAbstractGradient>();
            } else if (gradientType == "segment") {
                gradient = KoSegmentGradient::fromXML(gradientElement).clone().dynamicCast<KoAbstractGradient>();
            }
            if (gradient) {
                gradient->setName(gradientElement.attribute("name", ""));
                gradient->setValid(true);
                return gradient;
            }
        }
    }
    return defaultGradient();
}

QPair<QPointF, QPointF> KisGradientGeneratorConfiguration::absoluteCartesianPositionsInPixels(int width, int height) const
{
    QPointF startPosition(
        convertUnitsToPixels(startPositionX(), startPositionXUnits(), width, height),
        convertUnitsToPixels(startPositionY(), startPositionYUnits(), width, height)
    );
    QPointF endPosition;

    if (endPositionCoordinateSystem() == CoordinateSystemPolar) {
        qreal angle = endPositionAngle() * M_PI / 180.0;
        qreal distance = convertUnitsToPixels(endPositionDistance(), endPositionDistanceUnits(), width, height);
        endPosition = startPosition + distance * QPointF(std::cos(angle), -std::sin(angle));
    } else {
        endPosition = QPointF(
            convertUnitsToPixels(endPositionX(), endPositionXUnits(), width, height),
            convertUnitsToPixels(endPositionY(), endPositionYUnits(), width, height)
        );
        endPosition += QPointF(
            endPositionXPositioning() == PositioningRelative ? startPosition.x() : 0,
            endPositionYPositioning() == PositioningRelative ? startPosition.y() : 0
        );
    }

    return QPair<QPointF, QPointF>(startPosition, endPosition);
}

void KisGradientGeneratorConfiguration::setShape(KisGradientPainter::enumGradientShape newShape)
{
    setProperty("shape", shapeToString(newShape));
}

void KisGradientGeneratorConfiguration::setRepeat(KisGradientPainter::enumGradientRepeat newRepeat)
{
    setProperty("repeat", repeatToString(newRepeat));
}

void KisGradientGeneratorConfiguration::setAntiAliasThreshold(qreal newAntiAliasThreshold)
{
    setProperty("antialias_threshold", newAntiAliasThreshold);
}

void KisGradientGeneratorConfiguration::setDither(bool newDither)
{
    setProperty("dither", newDither);
}

void KisGradientGeneratorConfiguration::setReverse(bool newReverse)
{
    setProperty("reverse", newReverse);
}

void KisGradientGeneratorConfiguration::setStartPositionX(qreal newStartPositionX)
{
    setProperty("start_position_x", newStartPositionX);
}

void KisGradientGeneratorConfiguration::setStartPositionY(qreal newStartPositionY)
{
    setProperty("start_position_y", newStartPositionY);
}

void KisGradientGeneratorConfiguration::setStartPositionXUnits(SpatialUnits newStartPositionXUnits)
{
    setProperty("start_position_x_units", spatialUnitsToString(newStartPositionXUnits));
}

void KisGradientGeneratorConfiguration::setStartPositionYUnits(SpatialUnits newStartPositionYUnits)
{
    setProperty("start_position_y_units", spatialUnitsToString(newStartPositionYUnits));
}

void KisGradientGeneratorConfiguration::setEndPositionCoordinateSystem(CoordinateSystem newEndPositionCoordinateSystem)
{
    setProperty("end_positiom_coordinate_system", coordinateSystemToString(newEndPositionCoordinateSystem));
}

void KisGradientGeneratorConfiguration::setEndPositionX(qreal newEndPositionX)
{
    setProperty("end_position_x", newEndPositionX);
}

void KisGradientGeneratorConfiguration::setEndPositionY(qreal newEndPositionY)
{
    setProperty("end_position_y", newEndPositionY);
}

void KisGradientGeneratorConfiguration::setEndPositionXUnits(SpatialUnits newEndPositionXUnits)
{
    setProperty("end_position_x_units", spatialUnitsToString(newEndPositionXUnits));
}

void KisGradientGeneratorConfiguration::setEndPositionYUnits(SpatialUnits newEndPositionYUnits)
{
    setProperty("end_position_y_units", spatialUnitsToString(newEndPositionYUnits));
}

void KisGradientGeneratorConfiguration::setEndPositionXPositioning(Positioning newEndPositionXPositioning)
{
    setProperty("end_position_x_positioning", positioningToString(newEndPositionXPositioning));
}

void KisGradientGeneratorConfiguration::setEndPositionYPositioning(Positioning newEndPositionYPositioning)
{
    setProperty("end_position_y_positioning", positioningToString(newEndPositionYPositioning));
}

void KisGradientGeneratorConfiguration::setEndPositionAngle(qreal newEndPositionAngle)
{
    setProperty("end_position_angle", newEndPositionAngle);
}

void KisGradientGeneratorConfiguration::setEndPositionDistance(qreal newEndPositionDistance)
{
    setProperty("end_position_distance", newEndPositionDistance);
}

void KisGradientGeneratorConfiguration::setEndPositionDistanceUnits(SpatialUnits newEndPositionDistanceUnits)
{
    setProperty("end_position_distance_units", spatialUnitsToString(newEndPositionDistanceUnits));
}

void KisGradientGeneratorConfiguration::setGradient(KoAbstractGradientSP newGradient)
{
    if (!newGradient) {
        setProperty("gradient", "");
        return;
    }
    
    QDomDocument document;
    QDomElement gradientElement = document.createElement("gradient");
    gradientElement.setAttribute("name", newGradient->name());

    if (dynamic_cast<KoStopGradient*>(newGradient.data())) {
        KoStopGradient *gradient = dynamic_cast<KoStopGradient*>(newGradient.data());
        gradient->toXML(document, gradientElement);
    } else if (dynamic_cast<KoSegmentGradient*>(newGradient.data())) {
        KoSegmentGradient *gradient = dynamic_cast<KoSegmentGradient*>(newGradient.data());
        gradient->toXML(document, gradientElement);
    }

    document.appendChild(gradientElement);
    setProperty("gradient", document.toString());
}

void KisGradientGeneratorConfiguration::setDefaults()
{
    setShape(defaultShape());
    setRepeat(defaultRepeat());
    setAntiAliasThreshold(defaultAntiAliasThreshold());
    setReverse(defaultReverse());
    setStartPositionX(defaultStartPositionX());
    setStartPositionY(defaultStartPositionY());
    setStartPositionXUnits(defaultStartPositionXUnits());
    setStartPositionYUnits(defaultStartPositionYUnits());
    setEndPositionCoordinateSystem(defaultEndPositionCoordinateSystem());
    setEndPositionX(defaultEndPositionX());
    setEndPositionY(defaultEndPositionY());
    setEndPositionXUnits(defaultEndPositionXUnits());
    setEndPositionYUnits(defaultEndPositionYUnits());
    setEndPositionXPositioning(defaultEndPositionXPositioning());
    setEndPositionYPositioning(defaultEndPositionYPositioning());
    setEndPositionAngle(defaultEndPositionAngle());
    setEndPositionDistance(defaultEndPositionDistance());
    setEndPositionDistanceUnits(defaultEndPositionDistanceUnits());
    setGradient(defaultGradient());
}
