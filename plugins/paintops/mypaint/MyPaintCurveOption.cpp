/*
 * SPDX-FileCopyrightText: 2008 Boudewijn Rempt <boud@valdyas.org>
 * SPDX-FileCopyrightText: 2011 Silvio Heinrich <plassy@web.de>
 * SPDX-FileCopyrightText: 2020 Ashwin Dhakaita <ashwingpdhakaita@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <QFile>
#include <QDomNode>
#include <QJsonObject>
#include <QJsonDocument>
#include <kis_paintop_lod_limitations.h>

#include <libmypaint/mypaint-brush.h>
#include <libmypaint/mypaint-config.h>

#include "MyPaintPaintOpPreset.h"
#include "MyPaintPaintOpOption.h"
#include "MyPaintCurveOption.h"


struct MyPaintBrush;
KisMyPaintCurveOption::KisMyPaintCurveOption(const KoID &id, KisPaintOpOption::PaintopCategory category, bool checked, qreal value, qreal min, qreal max)
    : KisCurveOption(id, category, true, value, min, max)
{            
    Q_UNUSED(checked);
    m_checkable = false;
    m_checked = true;
    m_useCurve = true;
    m_useSameCurve = false;
    m_sensorMap.clear();

    Q_FOREACH (const DynamicSensorType sensorType, this->sensorsTypes()) {
        KisDynamicSensorSP sensor = type2Sensor(sensorType, m_id.id());
        sensor->setActive(false);
        replaceSensor(sensor);
    }
    m_sensorMap[MYPAINT_PRESSURE]->setActive(true);

    setValueRange(min, max);
    setValue(value);

    m_commonCurve = defaultCurve();
}

KisMyPaintCurveOption::~KisMyPaintCurveOption()
{
}

void KisMyPaintCurveOption::writeOptionSetting(KisPropertiesConfigurationSP setting) const
{    
    if (m_checkable) {
        setting->setProperty("Pressure" + m_id.id(), isChecked());
    }

    QJsonDocument doc = QJsonDocument::fromJson(setting->getProperty(MYPAINT_JSON).toByteArray());
    QJsonObject brush_json = doc.object();
    QVariantMap map = brush_json.toVariantMap();
    QVariantMap settings_map = map["settings"].toMap();
    QVariantMap name_map = settings_map[m_id.id()].toMap();
    QVariantMap inputs_map = name_map["inputs"].toMap();

    Q_FOREACH(KisDynamicSensorSP val, m_sensorMap.values()) {

        KisMyPaintBrushOption *option = dynamic_cast<KisMyPaintBrushOption*>(val.data());
        QVariantList pointsList;
        QList<QPointF> curve_points = option->getControlPoints();

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
    settings_map[m_id.id()] = name_map;
    map["settings"] = settings_map;

    QJsonObject resultant_json = QJsonObject::fromVariantMap(map);
    QJsonDocument doc2(resultant_json);

    setting->setProperty(MYPAINT_JSON, doc2.toJson());

    setting->setProperty(m_id.id() + "UseCurve", m_useCurve);
    setting->setProperty(m_id.id() + "UseSameCurve", m_useSameCurve);
    setting->setProperty(m_id.id() + "Value", m_value);
    setting->setProperty(m_id.id() + "curveMode", m_curveMode);
    setting->setProperty(m_id.id() + "commonCurve", QVariant::fromValue(m_commonCurve));
}

void KisMyPaintCurveOption::readOptionSetting(KisPropertiesConfigurationSP setting)
{
    readNamedOptionSetting(m_id.id(), setting);
}

void KisMyPaintCurveOption::readNamedOptionSetting(const QString& prefix, const KisPropertiesConfigurationSP setting)
{
    if (!setting) return;        

    KisCubicCurve commonCurve = m_commonCurve;

    if (m_checkable) {
        setChecked(setting->getBool("Pressure" + prefix, false));
    }    

    m_sensorMap.clear();    

    Q_FOREACH (const DynamicSensorType sensorType, this->sensorsTypes()) {
        replaceSensor(type2Sensor(sensorType, m_id.id()));
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

            option->setCurveFromPoints(points);
        }

        if (!setting->getProperty(m_id.id() + option->identifier() + "XMIN").isNull())
            option->setXRangeMin(setting->getProperty(m_id.id() + option->identifier() + "XMIN").toReal());

        if (!setting->getProperty(m_id.id() + option->identifier() + "XMAX").isNull())
            option->setXRangeMax(setting->getProperty(m_id.id() + option->identifier() + "XMAX").toReal());

        if (!setting->getProperty(m_id.id() + option->identifier() + "YMIN").isNull())
            option->setYRangeMin(setting->getProperty(m_id.id() + option->identifier() + "YMIN").toReal());

        if (!setting->getProperty(m_id.id() + option->identifier() + "YMAX").isNull())
            option->setYRangeMax(setting->getProperty(m_id.id() + option->identifier() + "YMAX").toReal());

        replaceSensor(option);
        option->setActive(points.size()>0);
    }

    m_useSameCurve = setting->getBool(m_id.id() + "UseSameCurve", false);

    Q_FOREACH(KisDynamicSensorSP sensor, m_sensorMap.values()) {
        commonCurve = sensor->curve();
    }

    m_useCurve = setting->getBool(m_id.id() + "UseCurve", true);
    if (m_useSameCurve) {
        m_commonCurve = setting->getCubicCurve(prefix + "commonCurve", commonCurve);
    }

    if (activeSensors().size() == 0) {
        m_useCurve = false;
        m_sensorMap[MYPAINT_PRESSURE]->setActive(true);
    }  
    if(!m_useCurve){
        emit unCheckUseCurve();
    }

    firstRead = false;
    m_value = setting->getDouble(m_id.id() + "Value", m_maxValue);
    m_curveMode = setting->getInt(m_id.id() + "curveMode");
    mypaint_brush_unref(brush);
}

void KisMyPaintCurveOption::lodLimitations(KisPaintopLodLimitations *l) const {

    if(m_sensorMap[MYPAINT_RANDOM]->isActive()) {
        l->blockers << KoID("Random Sensor Active", i18nc("PaintOp instant preview limitation", "Random Sensor Active, consider disabling Instant Preview"));
    }

    if (m_id.id() == "offset_by_random" && (m_value > 0.05 || m_value < -0.05)) {
        l->blockers << KoID("Offset by Random", i18nc("PaintOp instant preview limitation", "Offset by Random, consider disabling Instant Preview"));
    }

    if (m_id.id() == "radius_by_random" && qRound(m_value) >= 0.05) {
        l->blockers << KoID("Radius by Random", i18nc("PaintOp instant preview limitation", "Radius by Random, consider disabling Instant Preview"));
    }
}

MyPaintBrushSetting KisMyPaintCurveOption::currentSetting() {
    if (m_id.id() == "eraser")
        return MYPAINT_BRUSH_SETTING_ERASER;
    else if (m_id.id() == "opaque")
        return MYPAINT_BRUSH_SETTING_OPAQUE;
    else if (m_id.id() == "smudge")
        return MYPAINT_BRUSH_SETTING_SMUDGE;
    else if (m_id.id() == "color_h")
        return MYPAINT_BRUSH_SETTING_COLOR_H;
    else if (m_id.id() == "color_s")
        return MYPAINT_BRUSH_SETTING_COLOR_S;
    else if (m_id.id() == "color_v")
        return MYPAINT_BRUSH_SETTING_COLOR_V;
    else if (m_id.id() == "colorize")
        return MYPAINT_BRUSH_SETTING_COLORIZE;
    else if (m_id.id() == "hardness")
        return MYPAINT_BRUSH_SETTING_HARDNESS;
    else if (m_id.id() == "speed1_gamma")
        return MYPAINT_BRUSH_SETTING_SPEED1_GAMMA;
    else if (m_id.id() == "speed2_gamma")
        return MYPAINT_BRUSH_SETTING_SPEED2_GAMMA;
    else if (m_id.id() == "anti_aliasing")
        return MYPAINT_BRUSH_SETTING_ANTI_ALIASING;
    else if (m_id.id() == "restore_color")
        return MYPAINT_BRUSH_SETTING_RESTORE_COLOR;
    else if (m_id.id() == "slow_tracking")
        return MYPAINT_BRUSH_SETTING_SLOW_TRACKING;
    else if (m_id.id() == "smudge_length")
        return MYPAINT_BRUSH_SETTING_SMUDGE_LENGTH;
    else if (m_id.id() == "snap_to_pixel")
        return MYPAINT_BRUSH_SETTING_SNAP_TO_PIXEL;
    else if (m_id.id() == "change_color_h")
        return MYPAINT_BRUSH_SETTING_CHANGE_COLOR_H;
    else if (m_id.id() == "change_color_l")
        return MYPAINT_BRUSH_SETTING_CHANGE_COLOR_L;
    else if (m_id.id() == "change_color_v")
        return MYPAINT_BRUSH_SETTING_CHANGE_COLOR_V;
    else if (m_id.id() == "tracking_noise")
        return MYPAINT_BRUSH_SETTING_TRACKING_NOISE;
    else if (m_id.id() == "dabs_per_second")
        return MYPAINT_BRUSH_SETTING_DABS_PER_SECOND;
    else if (m_id.id() == "offset_by_speed")
        return MYPAINT_BRUSH_SETTING_OFFSET_BY_SPEED;
    else if (m_id.id() == "opaque_multiply")
        return MYPAINT_BRUSH_SETTING_OPAQUE_MULTIPLY;
    else if (m_id.id() == "speed1_slowness")
        return MYPAINT_BRUSH_SETTING_SPEED1_SLOWNESS;
    else if (m_id.id() == "speed2_slowness")
        return MYPAINT_BRUSH_SETTING_SPEED2_SLOWNESS;
    else if (m_id.id() == "stroke_holdtime")
        return MYPAINT_BRUSH_SETTING_STROKE_HOLDTIME;
    else if (m_id.id() == "direction_filter")
        return MYPAINT_BRUSH_SETTING_DIRECTION_FILTER;
    else if (m_id.id() == "offset_by_random")
        return MYPAINT_BRUSH_SETTING_OFFSET_BY_RANDOM;
    else if (m_id.id() == "opaque_linearize")
        return MYPAINT_BRUSH_SETTING_OPAQUE_LINEARIZE;
    else if (m_id.id() == "radius_by_random")
        return MYPAINT_BRUSH_SETTING_RADIUS_BY_RANDOM;
    else if (m_id.id() == "stroke_threshold")
        return MYPAINT_BRUSH_SETTING_STROKE_THRESHOLD;
    else if (m_id.id() == "pressure_gain_log")
        return MYPAINT_BRUSH_SETTING_PRESSURE_GAIN_LOG;
    else if (m_id.id() == "smudge_radius_log")
        return MYPAINT_BRUSH_SETTING_SMUDGE_RADIUS_LOG;
    else if (m_id.id() == "change_color_hsl_s")
        return MYPAINT_BRUSH_SETTING_CHANGE_COLOR_HSL_S;
    else if (m_id.id() == "change_color_hsv_s")
        return MYPAINT_BRUSH_SETTING_CHANGE_COLOR_HSV_S;
    else if (m_id.id() == "radius_logarithmic")
        return MYPAINT_BRUSH_SETTING_RADIUS_LOGARITHMIC;
    else if (m_id.id() == "elliptical_dab_angle")
        return MYPAINT_BRUSH_SETTING_ELLIPTICAL_DAB_ANGLE;
    else if (m_id.id() == "elliptical_dab_ratio")
        return MYPAINT_BRUSH_SETTING_ELLIPTICAL_DAB_RATIO;
    else if (m_id.id() == "custom_input_slowness")
        return MYPAINT_BRUSH_SETTING_CUSTOM_INPUT_SLOWNESS;
    else if (m_id.id() == "custom_input")
        return MYPAINT_BRUSH_SETTING_CUSTOM_INPUT;
    else if (m_id.id() == "dabs_per_basic_radius")
        return MYPAINT_BRUSH_SETTING_DABS_PER_BASIC_RADIUS;
    else if (m_id.id() == "slow_tracking_per_dab")
        return MYPAINT_BRUSH_SETTING_SLOW_TRACKING_PER_DAB;
    else if (m_id.id() == "dabs_per_actual_radius")
        return MYPAINT_BRUSH_SETTING_DABS_PER_ACTUAL_RADIUS;
    else if (m_id.id() == "offset_by_speed_slowness")
        return MYPAINT_BRUSH_SETTING_OFFSET_BY_SPEED_SLOWNESS;
    else if (m_id.id() == "stroke_duration_logarithmic")
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
         << MYPAINT_BRUSH_INPUT_TILT_ASCENSION
         << MYPAINT_BRUSH_INPUT_CUSTOM;

    return list;
}

QList<KoID> KisMyPaintCurveOption::sensorsIds()
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

DynamicSensorType KisMyPaintCurveOption::id2Type(const KoID &id)
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

KisDynamicSensorSP KisMyPaintCurveOption::id2Sensor(const KoID& id, const QString &parentOptionName)
{
    Q_UNUSED(parentOptionName);
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

QList<DynamicSensorType> KisMyPaintCurveOption::sensorsTypes()
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

KisDynamicSensorSP KisMyPaintCurveOption::type2Sensor(DynamicSensorType sensorType, const QString &parentOptionName)
{
    Q_UNUSED(parentOptionName);
    return new KisMyPaintBrushOption(sensorType);
}
