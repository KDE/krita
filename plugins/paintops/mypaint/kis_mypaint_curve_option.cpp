#include "kis_mypaint_curve_option.h"

#include "kis_my_paintop_option.h"
#include "kis_mypaint_brush.h"
#include <libmypaint/mypaint-brush.h>
#include <libmypaint/mypaint-config.h>

#include <QDomNode>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>

struct MyPaintBrush;
KisMyPaintCurveOption::KisMyPaintCurveOption(const QString& name, KisPaintOpOption::PaintopCategory category,
                               bool checked, qreal value, qreal min, qreal max)
    : m_name(name)
    , m_category(category)
    , m_checkable(false)
    , m_checked(true)
    , m_useCurve(true)
    , m_useSameCurve(false)
    , m_separateCurveValue(false)
    , m_curveMode(0)
{
    Q_FOREACH (const MyPaintBrushOptionType sensorType, KisMyPaintBrushOption::sensorsTypes()) {
        KisDynamicOptionSP sensor = KisMyPaintBrushOption::type2Sensor(sensorType, m_name);
        sensor->setActive(false);
        replaceSensor(sensor);
    }
    m_sensorMap[PRESSURE]->setActive(true);

    setValueRange(min, max);
    setValue(value);


    m_commonCurve = defaultCurve();
}

KisMyPaintCurveOption::~KisMyPaintCurveOption()
{
}

const QString& KisMyPaintCurveOption::name() const
{
    return m_name;
}

KisPaintOpOption::PaintopCategory KisMyPaintCurveOption::category() const
{
    return m_category;
}

qreal KisMyPaintCurveOption::minValue() const
{
    return m_minValue;
}

qreal KisMyPaintCurveOption::maxValue() const
{
    return m_maxValue;
}

qreal KisMyPaintCurveOption::value() const
{
    return m_value;
}

void KisMyPaintCurveOption::resetAllSensors()
{
    Q_FOREACH (KisDynamicOptionSP sensor, m_sensorMap.values()) {
        if (sensor->isActive()) {
            sensor->reset();
        }
    }
}

void KisMyPaintCurveOption::writeOptionSetting(KisPropertiesConfigurationSP setting)
{    
    if (m_checkable) {
        setting->setProperty("Pressure" + m_name, isChecked());
    }

    QJsonDocument doc = QJsonDocument::fromJson(setting->getProperty(MYPAINT_JSON).toByteArray());
    QJsonObject brush_json = doc.object();
    QVariantMap map = brush_json.toVariantMap();
    QVariantMap settings_map = map["settings"].toMap();
    QVariantMap name_map = settings_map[m_name].toMap();
    QVariantMap inputs_map = name_map["inputs"].toMap();

    Q_FOREACH(KisDynamicOptionSP option, m_sensorMap.values()) {

        QVariantList pointsList;
        QList<QPointF> curve_points = option->curve().points();

        if(!option->isActive()){
            inputs_map.remove(option->id(option->sensorType()));
            continue;
        }

        for(int i=0; i<curve_points.size(); i++) {
            QVariantList point;
            point << curve_points[i].x() << curve_points[i].y();
            pointsList.push_back(point);
        }

        inputs_map[option->id(option->sensorType())] = pointsList;
    }

    if(!m_useCurve || activeSensors().size()==0)
        inputs_map.clear();

    name_map["inputs"] = inputs_map;    
    settings_map[m_name] = name_map;
    map["settings"] = settings_map;

    QJsonObject resultant_json = QJsonObject::fromVariantMap(map);
    QJsonDocument doc2(resultant_json);

    setting->setProperty(MYPAINT_JSON, doc2.toJson());
    //qDebug() << doc2.toJson();
//    QIODevice *dev = new QFile("/home/ashwin-dhakaita/b009.myb");
//    dev->open(QIODevice::ReadWrite);
//    dev->write(doc2.toJson());
//    dev->close();

    setting->setProperty(m_name + "UseCurve", m_useCurve);
    setting->setProperty(m_name + "UseSameCurve", m_useSameCurve);
    setting->setProperty(m_name + "Value", m_value);
    setting->setProperty(m_name + "curveMode", m_curveMode);
    setting->setProperty(m_name + "commonCurve", qVariantFromValue(m_commonCurve));        
}

void KisMyPaintCurveOption::readOptionSetting(KisPropertiesConfigurationSP setting)
{
    readNamedOptionSetting(m_name, setting);
}

void KisMyPaintCurveOption::lodLimitations(KisPaintopLodLimitations *l) const
{
    Q_UNUSED(l);
}

int KisMyPaintCurveOption::intMinValue() const
{
    return 0;
}

int KisMyPaintCurveOption::intMaxValue() const
{
    return 100;
}

QString KisMyPaintCurveOption::valueSuffix() const
{
    return i18n("%");
}

void KisMyPaintCurveOption::readNamedOptionSetting(const QString& prefix, const KisPropertiesConfigurationSP setting)
{     
    if (!setting) return;        

    KisCubicCurve commonCurve = m_commonCurve;

    if (m_checkable) {
        setChecked(setting->getBool("Pressure" + prefix, false));
    }    

    m_sensorMap.clear();

    Q_FOREACH (const MyPaintBrushOptionType sensorType, KisMyPaintBrushOption::sensorsTypes()) {
        replaceSensor(KisMyPaintBrushOption::type2Sensor(sensorType, m_name));
    }

    MyPaintBrush *brush = mypaint_brush_new();
    mypaint_brush_from_string(brush, setting->getProperty(MYPAINT_JSON).toByteArray());

    Q_FOREACH(MyPaintBrushInput inp, inputList()) {

        QList<QPointF> points;
        for(int i=0; i<mypaint_brush_get_mapping_n(brush, currentSetting(), inp); i++) {
            float x, y;            
            mypaint_brush_get_mapping_point(brush, currentSetting(), inp, i, &x, &y);
            points << QPointF(x, y);
        }

        KisMyPaintBrushOption *option = new KisMyPaintBrushOption(KisMyPaintBrushOption::typeForInput(inp));

        if(points.size()) {

            KisCubicCurve curve(points);
            option->setCurve(curve);
        }

        replaceSensor(option);
        option->setActive(points.size()>0);

    }    

    m_useSameCurve = setting->getBool(m_name + "UseSameCurve", false);

    Q_FOREACH(KisDynamicOptionSP sensor, m_sensorMap.values()) {
        commonCurve = sensor->curve();
    }

    if (m_useSameCurve) {
        m_commonCurve = setting->getCubicCurve(prefix + "commonCurve", commonCurve);
    }

    if (activeSensors().size() == 0) {
//        m_sensorMap[PRESSURE]->setActive(true);
        if(m_useCurve){
            m_useCurve = false;
            emit checkUseCurve();
        }
        m_useCurve = false;
    }
    else {
        m_useCurve = true;
    }

    firstRead = false;
    m_value = setting->getDouble(m_name + "Value", m_maxValue);    
    //m_useCurve = setting->getBool(m_name + "UseCurve", true);
    m_curveMode = setting->getInt(m_name + "curveMode");    
}

void KisMyPaintCurveOption::replaceSensor(KisDynamicOptionSP s)
{
    Q_ASSERT(s);
    m_sensorMap[s->sensorType()] = s;
}

KisDynamicOptionSP KisMyPaintCurveOption::sensor(MyPaintBrushOptionType sensorType, bool active) const
{
    if (m_sensorMap.contains(sensorType)) {
        if (!active) {
            return m_sensorMap[sensorType];
        }
        else {
             if (m_sensorMap[sensorType]->isActive()) {
                 return m_sensorMap[sensorType];
             }
        }
    }
    return 0;
}


bool KisMyPaintCurveOption::isRandom() const
{
    return false;
}

bool KisMyPaintCurveOption::isCurveUsed() const
{
    return m_useCurve;
}

bool KisMyPaintCurveOption::isSameCurveUsed() const
{
    return m_useSameCurve;
}

int KisMyPaintCurveOption::getCurveMode() const
{
    return m_curveMode;
}

KisCubicCurve KisMyPaintCurveOption::getCommonCurve() const
{
    return m_commonCurve;
}

void KisMyPaintCurveOption::setSeparateCurveValue(bool separateCurveValue)
{
    m_separateCurveValue = separateCurveValue;
}

bool KisMyPaintCurveOption::isCheckable()
{
    return m_checkable;
}

bool KisMyPaintCurveOption::isChecked() const
{
    return m_checked;
}

void KisMyPaintCurveOption::setChecked(bool checked)
{
    m_checked = checked;
}

void KisMyPaintCurveOption::setCurveUsed(bool useCurve)
{
    m_useCurve = useCurve;
}

void KisMyPaintCurveOption::setCurveMode(int mode)
{
    m_curveMode = mode;
}

void KisMyPaintCurveOption::setUseSameCurve(bool useSameCurve)
{
    m_useSameCurve = useSameCurve;
}

void KisMyPaintCurveOption::setCommonCurve(KisCubicCurve curve)
{
    m_commonCurve = curve;
}

void KisMyPaintCurveOption::setCurve(MyPaintBrushOptionType sensorType, bool useSameCurve, const KisCubicCurve &curve)
{
    if (useSameCurve == m_useSameCurve) {
        if (useSameCurve) {
            m_commonCurve = curve;
        }
        else {
            KisDynamicOptionSP s = sensor(sensorType, false);
            if (s) {
                s->setCurve(curve);
            }

        }
    }
    else {
        if (!m_useSameCurve && useSameCurve) {
            m_commonCurve = curve;
        }
        else { //if (m_useSameCurve && !useSameCurve)
            KisDynamicOptionSP s = 0;
            // And set the current sensor to the current curve
            if (!m_sensorMap.contains(sensorType)) {
                s = KisMyPaintBrushOption::type2Sensor(sensorType, m_name);
            } else {
                KisDynamicOptionSP s = sensor(sensorType, false);
            }
            if (s) {
                s->setCurve(curve);
            }

        }
        m_useSameCurve = useSameCurve;
    }
}

void KisMyPaintCurveOption::setValueRange(qreal min, qreal max)
{
    m_minValue = qMin(min, max);
    m_maxValue = qMax(min, max);
}

void KisMyPaintCurveOption::setValue(qreal value)
{
    m_value = qBound(m_minValue, value, m_maxValue);
}

KisMyPaintCurveOption::ValueComponents KisMyPaintCurveOption::computeValueComponents(const KisPaintInformation& info) const
{
    ValueComponents components;

    if (m_useCurve) {
        QMap<MyPaintBrushOptionType, KisDynamicOptionSP>::const_iterator i;
        QList<double> sensorValues;
        for (i = m_sensorMap.constBegin(); i != m_sensorMap.constEnd(); ++i) {
            KisDynamicOptionSP s(i.value());

            if (s->isActive()) {
                qreal valueFromCurve = m_useSameCurve ? s->parameter(info, m_commonCurve, true) : s->parameter(info);
                if (s->isAdditive()) {
                    components.additive += valueFromCurve;
                    components.hasAdditive = true;
                } else if (s->isAbsoluteRotation()) {
                    components.absoluteOffset = valueFromCurve;
                    components.hasAbsoluteOffset =true;
                } else {
                    sensorValues << valueFromCurve;
                    components.hasScaling = true;
                }
            }
        }

        if (sensorValues.count() == 1) {
            components.scaling = sensorValues.first();
        } else {

            if (m_curveMode == 1){           // add
                components.scaling = 0;
                double i;
                foreach (i, sensorValues) {
                    components.scaling += i;
                }
            } else if (m_curveMode == 2){    //max
                components.scaling = *std::max_element(sensorValues.begin(), sensorValues.end());

            } else if (m_curveMode == 3){    //min
                components.scaling = *std::min_element(sensorValues.begin(), sensorValues.end());

            } else if (m_curveMode == 4){    //difference
                double max = *std::max_element(sensorValues.begin(), sensorValues.end());
                double min = *std::min_element(sensorValues.begin(), sensorValues.end());
                components.scaling = max-min;

            } else {                         //multuply - default
                double i;
                foreach (i, sensorValues) {
                    components.scaling *= i;
                }
            }
        }

    }

    if (!m_separateCurveValue) {
        components.constant = m_value;
    }

    components.minSizeLikeValue = m_minValue;
    components.maxSizeLikeValue = m_maxValue;

    return components;
}

qreal KisMyPaintCurveOption::computeSizeLikeValue(const KisPaintInformation& info) const
{
    const ValueComponents components = computeValueComponents(info);
    return components.sizeLikeValue();
}

qreal KisMyPaintCurveOption::computeRotationLikeValue(const KisPaintInformation& info, qreal baseValue, bool absoluteAxesFlipped) const
{
    const ValueComponents components = computeValueComponents(info);
    return components.rotationLikeValue(baseValue, absoluteAxesFlipped);
}

KisCubicCurve KisMyPaintCurveOption::defaultCurve()
{
    QList<QPointF> points;
    // needs to be set to something, weird curve is better for debugging
    // it will be reset to the curve from the preset anyway though
    points.push_back(QPointF(0,0));
    points.push_back(QPointF(0.25,0.9));
    points.push_back(QPointF(0.5,0));
    points.push_back(QPointF(0.75,0.6));
    points.push_back(QPointF(1,0));
    return KisCubicCurve(points);
}

KisCubicCurve KisMyPaintCurveOption::emptyCurve()
{
    QList<QPointF> points;
    points.push_back(QPointF(0,0));
    points.push_back(QPointF(1,1));
    return KisCubicCurve(points);
}

QList<KisDynamicOptionSP> KisMyPaintCurveOption::sensors()
{
    //dbgKrita << "ID" << name() << "has" <<  m_sensorMap.count() << "Sensors of which" << sensorList.count() << "are active.";
    return m_sensorMap.values();
}

QList<KisDynamicOptionSP> KisMyPaintCurveOption::activeSensors() const
{
    QList<KisDynamicOptionSP> sensorList;
    Q_FOREACH (KisDynamicOptionSP sensor, m_sensorMap.values()) {
        if (sensor->isActive()) {
            sensorList << sensor;
        }
    }
    //dbgKrita << "ID" << name() << "has" <<  m_sensorMap.count() << "Sensors of which" << sensorList.count() << "are active.";
    return sensorList;
}

MyPaintBrushSetting KisMyPaintCurveOption::currentSetting() {

    if(m_name == "eraser")
        return MYPAINT_BRUSH_SETTING_ERASER;
    else if(m_name == "opaque")
        return MYPAINT_BRUSH_SETTING_OPAQUE;
    else if(m_name == "smudge")
        return MYPAINT_BRUSH_SETTING_SMUDGE;
    else if(m_name == "color_h")
        return MYPAINT_BRUSH_SETTING_COLOR_H;
    else if(m_name == "color_s")
        return MYPAINT_BRUSH_SETTING_COLOR_S;
    else if(m_name == "color_v")
        return MYPAINT_BRUSH_SETTING_COLOR_V;
    else if(m_name == "colorize")
        return MYPAINT_BRUSH_SETTING_COLORIZE;
    else if(m_name == "hardness")
        return MYPAINT_BRUSH_SETTING_HARDNESS;
    else if(m_name == "speed1_gamma")
        return MYPAINT_BRUSH_SETTING_SPEED1_GAMMA;
    else if(m_name == "speed2_gamma")
        return MYPAINT_BRUSH_SETTING_SPEED2_GAMMA;
    else if(m_name == "anti_aliasing")
        return MYPAINT_BRUSH_SETTING_ANTI_ALIASING;
    else if(m_name == "restore_color")
        return MYPAINT_BRUSH_SETTING_RESTORE_COLOR;
    else if(m_name == "slow_tracking")
        return MYPAINT_BRUSH_SETTING_SLOW_TRACKING;
    else if(m_name == "smudge_length")
        return MYPAINT_BRUSH_SETTING_SMUDGE_LENGTH;
    else if(m_name == "snap_to_pixel")
        return MYPAINT_BRUSH_SETTING_SNAP_TO_PIXEL;
    else if(m_name == "change_color_h")
        return MYPAINT_BRUSH_SETTING_CHANGE_COLOR_H;
    else if(m_name == "change_color_l")
        return MYPAINT_BRUSH_SETTING_CHANGE_COLOR_L;
    else if(m_name == "change_color_v")
        return MYPAINT_BRUSH_SETTING_CHANGE_COLOR_V;
    else if(m_name == "tracking_noise")
        return MYPAINT_BRUSH_SETTING_TRACKING_NOISE;
    else if(m_name == "dabs_per_second")
        return MYPAINT_BRUSH_SETTING_DABS_PER_SECOND;
    else if(m_name == "offset_by_speed")
        return MYPAINT_BRUSH_SETTING_OFFSET_BY_SPEED;
    else if(m_name == "opaque_multiply")
        return MYPAINT_BRUSH_SETTING_OPAQUE_MULTIPLY;
    else if(m_name == "speed1_slowness")
        return MYPAINT_BRUSH_SETTING_SPEED1_SLOWNESS;
    else if(m_name == "speed2_slowness")
        return MYPAINT_BRUSH_SETTING_SPEED2_SLOWNESS;
    else if(m_name == "stroke_holdtime")
        return MYPAINT_BRUSH_SETTING_STROKE_HOLDTIME;
    else if(m_name == "direction_filter")
        return MYPAINT_BRUSH_SETTING_DIRECTION_FILTER;
    else if(m_name == "offset_by_random")
        return MYPAINT_BRUSH_SETTING_OFFSET_BY_RANDOM;
    else if(m_name == "opaque_linearize")
        return MYPAINT_BRUSH_SETTING_OPAQUE_LINEARIZE;
    else if(m_name == "radius_by_random")
        return MYPAINT_BRUSH_SETTING_RADIUS_BY_RANDOM;
    else if(m_name == "stroke_threshold")
        return MYPAINT_BRUSH_SETTING_STROKE_THRESHOLD;
    else if(m_name == "pressure_gain_log")
        return MYPAINT_BRUSH_SETTING_PRESSURE_GAIN_LOG;
    else if(m_name == "smudge_radius_log")
        return MYPAINT_BRUSH_SETTING_SMUDGE_RADIUS_LOG;
    else if(m_name == "change_color_hsl_s")
        return MYPAINT_BRUSH_SETTING_CHANGE_COLOR_HSL_S;
    else if(m_name == "change_color_hsv_s")
        return MYPAINT_BRUSH_SETTING_CHANGE_COLOR_HSV_S;
    else if(m_name == "radius_logarithmic")
        return MYPAINT_BRUSH_SETTING_RADIUS_LOGARITHMIC;
    else if(m_name == "elliptical_dab_angle")
        return MYPAINT_BRUSH_SETTING_ELLIPTICAL_DAB_ANGLE;
    else if(m_name == "elliptical_dab_ratio")
        return MYPAINT_BRUSH_SETTING_ELLIPTICAL_DAB_RATIO;
    else if(m_name == "custom_input_slowness")
        return MYPAINT_BRUSH_SETTING_CUSTOM_INPUT_SLOWNESS;
    else if(m_name == "dabs_per_basic_radius")
        return MYPAINT_BRUSH_SETTING_DABS_PER_BASIC_RADIUS;
    else if(m_name == "slow_tracking_per_dab")
        return MYPAINT_BRUSH_SETTING_SLOW_TRACKING_PER_DAB;
    else if(m_name == "dabs_per_actual_radius")
        return MYPAINT_BRUSH_SETTING_DABS_PER_ACTUAL_RADIUS;
    else if(m_name == "offset_by_speed_slowness")
        return MYPAINT_BRUSH_SETTING_OFFSET_BY_SPEED_SLOWNESS;
    else if(m_name == "stroke_duration_logarithmic")
        return MYPAINT_BRUSH_SETTING_STROKE_DURATION_LOGARITHMIC;

    return MYPAINT_BRUSH_SETTING_ERASER;
}

QList<MyPaintBrushInput> KisMyPaintCurveOption::inputList() {

    QList<MyPaintBrushInput> list;

    list << MYPAINT_BRUSH_INPUT_PRESSURE
         << MYPAINT_BRUSH_INPUT_SPEED1
         << MYPAINT_BRUSH_INPUT_SPEED2
         << MYPAINT_BRUSH_INPUT_RANDOM
         << MYPAINT_BRUSH_INPUT_STROKE
         << MYPAINT_BRUSH_INPUT_DIRECTION
         << MYPAINT_BRUSH_INPUT_TILT_DECLINATION
         << MYPAINT_BRUSH_INPUT_TILT_ASCENSION;

    return list;
}
