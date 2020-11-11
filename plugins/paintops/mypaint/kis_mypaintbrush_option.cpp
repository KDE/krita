/*
 * Copyright (c) 2020 Ashwin Dhakaita <ashwingpdhakaita@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#include <qmath.h>
#include <QDomElement>

#include <kis_algebra_2d.h>
#include <kis_paint_information.h>
#include <libmypaint/mypaint-brush.h>

#include "kis_mypaintbrush_option.h"

using namespace std;

KisMyPaintBrushOption::KisMyPaintBrushOption(DynamicSensorType type)
    : KisDynamicSensor (type)       
{
    m_type = type;
    m_customCurve = false;
    m_active = false;
    m_length = -1;
    m_id = KisMyPaintBrushOption::id(type);
}

KisMyPaintBrushOption::~KisMyPaintBrushOption()
{
}

qreal KisMyPaintBrushOption::value(const KisPaintInformation &info)
{
    if (info.isHoveringMode()) return 1.0;

    const int currentValue =
        qMin(info.currentDabSeqNo(), m_length);

    return qreal(currentValue) / m_length;
}

KisDynamicSensorSP KisMyPaintBrushOption::id2Sensor(const KoID& id, const QString &parentOptionName)
{
    if(id.id()==Pressure.id())
        return new KisMyPaintBrushOption(MYPAINT_PRESSURE);
    else if(id.id()==FineSpeed.id())
        return new KisMyPaintBrushOption(MYPAINT_FINE_SPEED);
    else if(id.id()==GrossSpeed.id())
        return new KisMyPaintBrushOption(MYPAINT_GROSS_SPEED);
    else if(id.id()==Random.id())
        return new KisMyPaintBrushOption(MYPAINT_RANDOM);
    else if(id.id()==Stroke.id())
        return new KisMyPaintBrushOption(MYPAINT_STROKE);
    else if(id.id()==Direction.id())
        return new KisMyPaintBrushOption(MYPAINT_DIRECTION);
    else if(id.id()==Ascension.id())
        return new KisMyPaintBrushOption(MYPAINT_ASCENSION);
    else if(id.id()==Declination.id())
        return new KisMyPaintBrushOption(MYPAINT_DECLINATION);
    else if(id.id()==Custom.id())
        return new KisMyPaintBrushOption(MYPAINT_CUSTOM);
    else {
        return 0;
    }
}

DynamicSensorType KisMyPaintBrushOption::id2Type(const KoID &id)
{
    if (id.id() == Pressure.id()) {
        return MYPAINT_PRESSURE;
    }
    else if (id.id() == FineSpeed.id()) {
        return MYPAINT_FINE_SPEED;
    }
    else if (id.id() == GrossSpeed.id()) {
        return MYPAINT_GROSS_SPEED;
    }
    else if (id.id() == Random.id()) {
        return MYPAINT_RANDOM;
    }
    else if (id.id() == Stroke.id()) {
        return MYPAINT_STROKE;
    }
    else if (id.id() == Direction.id()) {
        return MYPAINT_DIRECTION;
    }
    else if (id.id() == Declination.id()) {
        return MYPAINT_DECLINATION;
    }
    else if (id.id() == Ascension.id()) {
        return MYPAINT_ASCENSION;
    }
    else if (id.id() == Custom.id()) {
        return MYPAINT_CUSTOM;
    }
    return UNKNOWN;
}

KisDynamicSensorSP KisMyPaintBrushOption::type2Sensor(DynamicSensorType sensorType, const QString &parentOptionName)
{
    return new KisMyPaintBrushOption(sensorType);
}

QString KisMyPaintBrushOption::minimumLabel(DynamicSensorType sensorType)
{
    switch (sensorType) {
    default:
        return i18n("0.0");
    }
}

QString KisMyPaintBrushOption::maximumLabel(DynamicSensorType sensorType, int max)
{
    switch (sensorType) {
    default:
        return i18n("1.0");
    };
}

QString KisMyPaintBrushOption::minimumXLabel() {

    return QString::number(curveXMin);
}

QString KisMyPaintBrushOption::minimumYLabel() {

    return QString::number(curveYMin);
}

QString KisMyPaintBrushOption::maximumXLabel() {

    return QString::number(curveXMax);
}

QString KisMyPaintBrushOption::maximumYLabel() {

    return QString::number(curveYMax);
}

int KisMyPaintBrushOption::minimumValue(DynamicSensorType sensorType)
{
    switch (sensorType) {

    default:
        return 0;
    }

}

int KisMyPaintBrushOption::maximumValue(DynamicSensorType sensorType, int max)
{
    switch (sensorType) {
    default:
        return 100;
    };
}

QString KisMyPaintBrushOption::valueSuffix(DynamicSensorType sensorType)
{
    switch (sensorType) {

    default:
        return i18n("%");
    };
}

QList<KoID> KisMyPaintBrushOption::sensorsIds()
{
    QList<KoID> ids;

    ids << Pressure
        << FineSpeed
        << GrossSpeed
        << Random
        << Stroke
        << Direction
        << Declination
        << Ascension
        << Custom;

    return ids;
}

QList<DynamicSensorType> KisMyPaintBrushOption::sensorsTypes()
{
    QList<DynamicSensorType> sensorTypes;
    sensorTypes
            << MYPAINT_PRESSURE
            << MYPAINT_FINE_SPEED
            << MYPAINT_GROSS_SPEED
            << MYPAINT_RANDOM
            << MYPAINT_STROKE
            << MYPAINT_DIRECTION
            << MYPAINT_DECLINATION
            << MYPAINT_ASCENSION
            << MYPAINT_CUSTOM;

    return sensorTypes;
}

DynamicSensorType KisMyPaintBrushOption::typeForInput(MyPaintBrushInput input)
{
    switch(input) {

        case MYPAINT_BRUSH_INPUT_PRESSURE:
            return MYPAINT_PRESSURE;
        case MYPAINT_BRUSH_INPUT_SPEED1:
            return MYPAINT_FINE_SPEED;
        case MYPAINT_BRUSH_INPUT_SPEED2:
            return MYPAINT_GROSS_SPEED;
        case MYPAINT_BRUSH_INPUT_RANDOM:
            return MYPAINT_RANDOM;
        case MYPAINT_BRUSH_INPUT_STROKE:
            return MYPAINT_STROKE;
        case MYPAINT_BRUSH_INPUT_DIRECTION:
            return MYPAINT_DIRECTION;
        case MYPAINT_BRUSH_INPUT_TILT_DECLINATION:
            return MYPAINT_DECLINATION;
        case MYPAINT_BRUSH_INPUT_TILT_ASCENSION:
            return MYPAINT_ASCENSION;
        case MYPAINT_BRUSH_INPUT_CUSTOM:
            return MYPAINT_CUSTOM;

        default:
            return MYPAINT_PRESSURE;
    }
}

MyPaintBrushInput KisMyPaintBrushOption::input()
{
    switch(m_type) {

        case MYPAINT_PRESSURE:
            return  MYPAINT_BRUSH_INPUT_PRESSURE;
        case MYPAINT_FINE_SPEED:
            return MYPAINT_BRUSH_INPUT_SPEED1;
        case MYPAINT_GROSS_SPEED:
            return MYPAINT_BRUSH_INPUT_SPEED2;
        case MYPAINT_RANDOM:
            return MYPAINT_BRUSH_INPUT_RANDOM;
        case MYPAINT_STROKE:
            return MYPAINT_BRUSH_INPUT_STROKE;
        case MYPAINT_DIRECTION:
            return MYPAINT_BRUSH_INPUT_DIRECTION;
        case MYPAINT_DECLINATION:
            return MYPAINT_BRUSH_INPUT_TILT_DECLINATION;
        case MYPAINT_ASCENSION:
            return MYPAINT_BRUSH_INPUT_TILT_ASCENSION;
        case MYPAINT_CUSTOM:
            return MYPAINT_BRUSH_INPUT_CUSTOM;

        default:
            return MYPAINT_BRUSH_INPUT_PRESSURE;
    }
}

qreal KisMyPaintBrushOption::getXRangeMin() {

    return curveXMin;
}

qreal KisMyPaintBrushOption::getXRangeMax() {

    return curveXMax;
}

qreal KisMyPaintBrushOption::getYRangeMin() {

    return curveYMin;
}

qreal KisMyPaintBrushOption::getYRangeMax() {

    return curveYMax;
}

void KisMyPaintBrushOption::setXRangeMin(qreal value) {

    curveXMin = value;
}

void KisMyPaintBrushOption::setXRangeMax(qreal value) {

    curveXMax = value;
}

void KisMyPaintBrushOption::setYRangeMin(qreal value) {

    curveYMin = value;
}

void KisMyPaintBrushOption::setYRangeMax(qreal value) {

    curveYMax = value;
}


QString KisMyPaintBrushOption::id(DynamicSensorType sensorType)
{
    switch (sensorType) {

    case MYPAINT_PRESSURE:
        return Pressure.id();
    case MYPAINT_FINE_SPEED:
        return FineSpeed.id();
    case MYPAINT_GROSS_SPEED:
        return GrossSpeed.id();
    case MYPAINT_RANDOM:
        return Random.id();
    case MYPAINT_DIRECTION:
        return Direction.id();
    case MYPAINT_ASCENSION:
        return Ascension.id();
    case MYPAINT_DECLINATION:
        return Declination.id();
    case MYPAINT_STROKE:
        return Stroke.id();
    case MYPAINT_CUSTOM:
        return Custom.id();

    default:
        return QString();
    };
}

void KisMyPaintBrushOption::setCurveFromPoints(QList<QPointF> points) {

    setRangeFromPoints(points);

    for (int i=0; i<points.size(); i++) {

        points[i] = scaleTo0_1(points[i]);
    }

    KisCubicCurve curve(points);
    setCurve(curve);
}

QList<QPointF> KisMyPaintBrushOption::getControlPoints() {

    QList<QPointF> curvePoints = curve().points();
    for(int i=0; i<curvePoints.size(); i++) {

         curvePoints[i] = scaleFrom0_1(curvePoints[i]);
    }

    return curvePoints;
}

QPointF KisMyPaintBrushOption::scaleTo0_1(QPointF point) {

    QPointF scaledPoint;
    scaledPoint.setX(scaleToRange(curveXMin, curveXMax, 0, 1, point.x()));
    scaledPoint.setY(scaleToRange(curveYMin, curveYMax, 0, 1, point.y()));

    return scaledPoint;
}

QPointF KisMyPaintBrushOption::scaleFrom0_1(QPointF point) {

    QPointF scaledPoint;
    scaledPoint.setX(scaleToRange(0, 1, curveXMin, curveXMax, point.x()));
    scaledPoint.setY(scaleToRange(0, 1, curveYMin, curveYMax, point.y()));

    return scaledPoint;
}

qreal KisMyPaintBrushOption::scaleToRange(qreal inMin, qreal inMax, qreal outMin, qreal outMax, qreal inValue) {

    qreal inRange = (inMax - inMin);
    qreal outRange = (outMax - outMin);
    qreal value = (((inValue - inMin) * outRange) / inRange) + outMin;

    return value;
}

void KisMyPaintBrushOption::setRangeFromPoints(QList<QPointF> points) {

    curveXMin = points[0].x();
    curveXMax = points[0].x();
    curveYMin = points[0].y();
    curveYMax = points[0].y();

    for(int i=1; i<points.size(); i++) {

        curveXMin = min(curveXMin, points[i].x());
        curveYMin = min(curveYMin, points[i].y());
        curveXMax = max(curveXMax, points[i].x());
        curveYMax = max(curveYMax, points[i].y());
    }

    if(abs(curveXMax-curveXMin) < 1)
        curveXMin = curveXMax - 1;

    if(abs(curveYMax-curveYMin) < 1)
        curveYMin = curveYMax - 1;
}
