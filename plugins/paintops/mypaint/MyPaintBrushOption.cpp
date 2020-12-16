/*
 * SPDX-FileCopyrightText: 2020 Ashwin Dhakaita <ashwingpdhakaita@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "MyPaintBrushOption.h"

#include <qmath.h>
#include <QDomElement>

#include <kis_algebra_2d.h>
#include <kis_paint_information.h>
#include <libmypaint/mypaint-brush.h>


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

QString KisMyPaintBrushOption::minimumLabel(DynamicSensorType sensorType)
{
    switch (sensorType) {
    default:
        return i18n("0.0");
    }
}

QString KisMyPaintBrushOption::maximumLabel(DynamicSensorType sensorType, int max)
{
    Q_UNUSED(max);
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
    Q_UNUSED(max);
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
