#include "kis_mypaintbrush_option.h"
#include <kis_paint_information.h>
#include <QDomElement>
#include <qmath.h>
#include <libmypaint/mypaint-brush.h>

#include "kis_algebra_2d.h"

using namespace std;

KisMyPaintBrushOption::KisMyPaintBrushOption(MyPaintBrushOptionType type)
    : m_length(-1)
    , m_type(type)
    , m_customCurve(false)
    , m_active(false)
{
}

KisMyPaintBrushOption::~KisMyPaintBrushOption()
{
}

QWidget* KisMyPaintBrushOption::createConfigurationWidget(QWidget* parent, QWidget*)
{
    Q_UNUSED(parent);
    return 0;
}

void KisMyPaintBrushOption::reset()
{
}

qreal KisMyPaintBrushOption::value(const KisPaintInformation &info)
{
    if (info.isHoveringMode()) return 1.0;

    const int currentValue =
        qMin(info.currentDabSeqNo(), m_length);

    return qreal(currentValue) / m_length;
}

KisDynamicOptionSP KisMyPaintBrushOption::id2Sensor(const KoID& id, const QString &parentOptionName)
{
    if(id.id()==Pressure.id())
        return new KisMyPaintBrushOption(PRESSURE);
    else if(id.id()==FineSpeed.id())
        return new KisMyPaintBrushOption(FINE_SPEED);
    else if(id.id()==GrossSpeed.id())
        return new KisMyPaintBrushOption(GROSS_SPEED);
    else if(id.id()==Random.id())
        return new KisMyPaintBrushOption(RANDOM);
    else if(id.id()==Stroke.id())
        return new KisMyPaintBrushOption(STROKE);
    else if(id.id()==Direction.id())
        return new KisMyPaintBrushOption(DIRECTION);
    else if(id.id()==Ascension.id())
        return new KisMyPaintBrushOption(ASCENSION);
    else if(id.id()==Declination.id())
        return new KisMyPaintBrushOption(DECLINATION);
    else if(id.id()==Custom.id())
        return new KisMyPaintBrushOption(CUSTOM);
    else {
        return 0;
    }
}

MyPaintBrushOptionType KisMyPaintBrushOption::id2Type(const KoID &id)
{
    if (id.id() == Pressure.id()) {
        return PRESSURE;
    }
    else if (id.id() == FineSpeed.id()) {
        return FINE_SPEED;
    }
    else if (id.id() == GrossSpeed.id()) {
        return GROSS_SPEED;
    }
    else if (id.id() == Random.id()) {
        return RANDOM;
    }
    else if (id.id() == Stroke.id()) {
        return STROKE;
    }
    else if (id.id() == Direction.id()) {
        return DIRECTION;
    }
    else if (id.id() == Declination.id()) {
        return DECLINATION;
    }
    else if (id.id() == Ascension.id()) {
        return ASCENSION;
    }
    else if (id.id() == Custom.id()) {
        return CUSTOM;
    }
    return UNKNOWN;
}

KisDynamicOptionSP KisMyPaintBrushOption::type2Sensor(MyPaintBrushOptionType sensorType, const QString &parentOptionName)
{
    return new KisMyPaintBrushOption(sensorType);
}

QString KisMyPaintBrushOption::minimumLabel(MyPaintBrushOptionType sensorType)
{
    switch (sensorType) {
    default:
        return i18n("0.0");
    }
}

QString KisMyPaintBrushOption::maximumLabel(MyPaintBrushOptionType sensorType, int max)
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

int KisMyPaintBrushOption::minimumValue(MyPaintBrushOptionType sensorType)
{
    switch (sensorType) {

    default:
        return 0;
    }

}

int KisMyPaintBrushOption::maximumValue(MyPaintBrushOptionType sensorType, int max)
{
    switch (sensorType) {
    default:
        return 100;
    };
}

QString KisMyPaintBrushOption::valueSuffix(MyPaintBrushOptionType sensorType)
{
    switch (sensorType) {

    default:
        return i18n("%");
    };
}

KisDynamicOptionSP KisMyPaintBrushOption::createFromXML(const QString& s, const QString &parentOptionName)
{
    QDomDocument doc;
    doc.setContent(s);
    QDomElement e = doc.documentElement();
    return createFromXML(e, parentOptionName);
}

KisDynamicOptionSP KisMyPaintBrushOption::createFromXML(const QDomElement& e, const QString &parentOptionName)
{
    QString id = e.attribute("id", "");
    KisDynamicOptionSP sensor = id2Sensor(id, parentOptionName);
    if (sensor) {
        sensor->fromXML(e);
    }
    return sensor;
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

QList<MyPaintBrushOptionType> KisMyPaintBrushOption::sensorsTypes()
{
    QList<MyPaintBrushOptionType> sensorTypes;
    sensorTypes
            << PRESSURE
            << FINE_SPEED
            << GROSS_SPEED
            << RANDOM
            << STROKE
            << DIRECTION
            << DECLINATION
            << ASCENSION
            << CUSTOM;

    return sensorTypes;
}

MyPaintBrushOptionType KisMyPaintBrushOption::typeForInput(MyPaintBrushInput input)
{
    switch(input) {

        case MYPAINT_BRUSH_INPUT_PRESSURE:
            return MyPaintBrushOptionType::PRESSURE;
        case MYPAINT_BRUSH_INPUT_SPEED1:
            return MyPaintBrushOptionType::FINE_SPEED;
        case MYPAINT_BRUSH_INPUT_SPEED2:
            return MyPaintBrushOptionType::GROSS_SPEED;
        case MYPAINT_BRUSH_INPUT_RANDOM:
            return MyPaintBrushOptionType::RANDOM;
        case MYPAINT_BRUSH_INPUT_STROKE:
            return MyPaintBrushOptionType::STROKE;
        case MYPAINT_BRUSH_INPUT_DIRECTION:
            return MyPaintBrushOptionType::DIRECTION;
        case MYPAINT_BRUSH_INPUT_TILT_DECLINATION:
            return MyPaintBrushOptionType::DECLINATION;
        case MYPAINT_BRUSH_INPUT_TILT_ASCENSION:
            return MyPaintBrushOptionType::ASCENSION;
        case MYPAINT_BRUSH_INPUT_CUSTOM:
            return MyPaintBrushOptionType::CUSTOM;

        default:
            return MyPaintBrushOptionType::PRESSURE;
    }
}

MyPaintBrushInput KisMyPaintBrushOption::input()
{
    switch(m_type) {

        case PRESSURE:
            return  MYPAINT_BRUSH_INPUT_PRESSURE;
        case FINE_SPEED:
            return MYPAINT_BRUSH_INPUT_SPEED1;
        case GROSS_SPEED:
            return MYPAINT_BRUSH_INPUT_SPEED2;
        case RANDOM:
            return MYPAINT_BRUSH_INPUT_RANDOM;
        case STROKE:
            return MYPAINT_BRUSH_INPUT_STROKE;
        case DIRECTION:
            return MYPAINT_BRUSH_INPUT_DIRECTION;
        case DECLINATION:
            return MYPAINT_BRUSH_INPUT_TILT_DECLINATION;
        case ASCENSION:
            return MYPAINT_BRUSH_INPUT_TILT_ASCENSION;
        case CUSTOM:
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


QString KisMyPaintBrushOption::id(MyPaintBrushOptionType sensorType)
{
    switch (sensorType) {

    case PRESSURE:
        return Pressure.id();
    case FINE_SPEED:
        return FineSpeed.id();
    case GROSS_SPEED:
        return GrossSpeed.id();
    case RANDOM:
        return Random.id();
    case DIRECTION:
        return Direction.id();
    case ASCENSION:
        return Ascension.id();
    case DECLINATION:
        return Declination.id();
    case STROKE:
        return Stroke.id();
    case CUSTOM:
        return Custom.id();

    default:
        return QString();
    };
}

QString KisMyPaintBrushOption::id() {

    return id(m_type);
}


void KisMyPaintBrushOption::toXML(QDomDocument& doc, QDomElement& elt) const
{
    elt.setAttribute("id", id(sensorType()));
    if (m_customCurve) {
        QDomElement curve_elt = doc.createElement("curve");
        QDomText text = doc.createTextNode(m_curve.toString());
        curve_elt.appendChild(text);
        elt.appendChild(curve_elt);
    }
}

void KisMyPaintBrushOption::fromXML(const QDomElement& e)
{
    Q_ASSERT(e.attribute("id", "") == id(sensorType()));
    m_customCurve = false;
    QDomElement curve_elt = e.firstChildElement("curve");
    if (!curve_elt.isNull()) {
        m_customCurve = true;
        m_curve.fromString(curve_elt.text());
    }
}

qreal KisMyPaintBrushOption::parameter(const KisPaintInformation& info)
{
    return parameter(info, m_curve, m_customCurve);
}

qreal KisMyPaintBrushOption::parameter(const KisPaintInformation& info, const KisCubicCurve curve, const bool customCurve)
{
    const qreal val = value(info);
    if (customCurve) {
        qreal scaledVal = isAdditive() ? additiveToScaling(val) : val;

        const QVector<qreal> transfer = curve.floatTransfer(256);
        scaledVal = KisCubicCurve::interpolateLinear(scaledVal, transfer);

        return isAdditive() ? scalingToAdditive(scaledVal) : scaledVal;
    }
    else {
        return val;
    }
}

void KisMyPaintBrushOption::setCurve(const KisCubicCurve& curve)
{
    m_customCurve = true;
    m_curve = curve;
}

void KisMyPaintBrushOption::setCurveFromPoints(QList<QPointF> points) {

    setRangeFromPoints(points);

    for (int i=0; i<points.size(); i++) {

        points[i] = scaleTo0_1(points[i]);
    }

    KisCubicCurve curve(points);
    setCurve(curve);
}

const KisCubicCurve& KisMyPaintBrushOption::curve() const
{
    return m_curve;
}

void KisMyPaintBrushOption::removeCurve()
{
    m_customCurve = false;
}

QList<QPointF> KisMyPaintBrushOption::getControlPoints() {

    QList<QPointF> curvePoints = curve().points();
    for(int i=0; i<curvePoints.size(); i++) {

         curvePoints[i] = scaleFrom0_1(curvePoints[i]);
    }

    return curvePoints;
}

bool KisMyPaintBrushOption::hasCustomCurve() const
{
    return m_customCurve;
}

bool KisMyPaintBrushOption::dependsOnCanvasRotation() const
{
    return true;
}

bool KisMyPaintBrushOption::isAdditive() const
{
    return false;
}

bool KisMyPaintBrushOption::isAbsoluteRotation() const
{
    return false;
}

void KisMyPaintBrushOption::setActive(bool active)
{
    m_active = active;
}

bool KisMyPaintBrushOption::isActive() const
{
    return m_active;
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

    curveXMax = curveXMax + 1;
    curveYMax = curveYMax + 1;
    curveXMin = curveXMin - 1;
    curveYMin = curveYMin - 1;
}
