/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "MyPaintSensorPack.h"

#include <KisCppQuirks.h>
#include "KisSensorData.h"
#include "kis_assert.h"

#include <KisDynamicSensorFactoryRegistry.h>
#include <KisSimpleDynamicSensorFactory.h>

#include <KisCurveOptionDataCommon.h>
#include <libmypaint/mypaint-brush-settings-gen.h>
#include <libmypaint/mypaint-brush.h>
#include <kis_algebra_2d.h>
#include <MyPaintCurveRangeModel.h>
#include <QJsonDocument>
#include <QJsonObject>
#include <KisStaticInitializer.h>

namespace detail {

KIS_DECLARE_STATIC_INITIALIZER {
    auto addFactory = [](const KoID &id,
                         int minimumValue,
                         int maximumValue,
                         const QString &minimumLabel,
                         const QString &maximumLabel,
                         const QString &valueSuffix) {
        KisDynamicSensorFactoryRegistry::instance()->add(new KisSimpleDynamicSensorFactory(id.id(),
                                                                                           minimumValue,
                                                                                           maximumValue,
                                                                                           minimumLabel,
                                                                                           maximumLabel,
                                                                                           valueSuffix));
    };

    addFactory(MyPaintPressureId, 0, 20, "", "", "");
    addFactory(MyPaintFineSpeedId, -20, 20, "", "", "");
    addFactory(MyPaintGrossSpeedId, -20, 20, "", "", "");
    addFactory(MyPaintRandomId,  0, 1, "", "", "");
    addFactory(MyPaintStrokeId, 0, 1, "", "", "");
    addFactory(MyPaintDirectionId, 0, 180, "", "", "");
    addFactory(MyPaintDeclinationId, 0, 90, "", "", i18n("%"));
    addFactory(MyPaintAscensionId, -180, 180, "", "", i18n("%"));
    addFactory(MyPaintCustomId, -20, 20, "", "", i18n("%"));
}

template <typename Data,
          typename SensorData =
              std::copy_const_t<Data,
                               KisSensorData>>
inline std::vector<SensorData*> sensors(Data *data)
{
    std::vector<SensorData*> result;

    result.reserve(9);

    result.push_back(&data->sensorPressure);
    result.push_back(&data->sensorFineSpeed);
    result.push_back(&data->sensorGrossSpeed);
    result.push_back(&data->sensorRandom);
    result.push_back(&data->sensorStroke);
    result.push_back(&data->sensorDirection);
    result.push_back(&data->sensorDeclination);
    result.push_back(&data->sensorAscension);
    result.push_back(&data->sensorCustom);

    return result;
}

} // namespace detail

MyPaintSensorDataWithRange::MyPaintSensorDataWithRange(const KoID &id)
    : KisSensorData(id)
{
    QVector<QPointF> points;

    if (id == MyPaintPressureId) {
        points = {{0,0}, {1,1}};
    } else if (id == MyPaintFineSpeedId) {
        points = {{0,0}, {4,1}};
    } else if (id == MyPaintGrossSpeedId) {
        points = {{0,0}, {4,1}};
    } else if (id == MyPaintRandomId) {
        points = {{0,0}, {1,1}};
    } else if (id == MyPaintStrokeId) {
        points = {{0,0}, {1,1}};
    } else if (id == MyPaintDirectionId) {
        points = {{0,0}, {180,1}};
    } else if (id == MyPaintDeclinationId) {
        points = {{0,0}, {90,1}};
    } else if (id == MyPaintAscensionId) {
        points = {{-180,0}, {180,1}};
    } else if (id == MyPaintCustomId) {
        points = {{-10,0}, {10,1}};
    } else {
        qWarning() << "MyPaintSensorDataWithRange: unknown sensor type:" << id;
        points = {{0,0}, {1,1}};
    }

    curve = KisCubicCurve(points).toString();
    curveRange = KisAlgebra2D::accumulateBounds(points);
    reshapeCurve();
}

QRectF MyPaintSensorDataWithRange::baseCurveRange() const
{
    return curveRange;
}

void MyPaintSensorDataWithRange::setBaseCurveRange(const QRectF &rect)
{
    curveRange = rect;
}

void MyPaintSensorDataWithRange::reset()
{
    *this = MyPaintSensorDataWithRange(id);
}

void MyPaintSensorDataWithRange::reshapeCurve()
{
    std::tie(curve, curveRange) = MyPaintCurveRangeModel::reshapeCurve(std::make_tuple(curve, curveRange));
}

MyPaintSensorData::MyPaintSensorData()
    : sensorPressure(MyPaintPressureId),
      sensorFineSpeed(MyPaintFineSpeedId),
      sensorGrossSpeed(MyPaintGrossSpeedId),
      sensorRandom(MyPaintRandomId),
      sensorStroke(MyPaintStrokeId),
      sensorDirection(MyPaintDirectionId),
      sensorDeclination(MyPaintDeclinationId),
      sensorAscension(MyPaintAscensionId),
      sensorCustom(MyPaintCustomId)
{
    sensorPressure.isActive = true;
}

KisSensorPackInterface * MyPaintSensorPack::clone() const
{
    return new MyPaintSensorPack(*this);
}

std::vector<const KisSensorData *> MyPaintSensorPack::constSensors() const
{
    return detail::sensors(&m_data);
}

std::vector<KisSensorData *> MyPaintSensorPack::sensors()
{
    return detail::sensors(&m_data);
}

const MyPaintSensorData& MyPaintSensorPack::constSensorsStruct() const 
{
    return m_data;
}

MyPaintSensorData& MyPaintSensorPack::sensorsStruct()
{
    return m_data;
}

bool MyPaintSensorPack::compare(const KisSensorPackInterface *rhs) const
{
    const MyPaintSensorPack *pack = dynamic_cast<const MyPaintSensorPack*>(rhs);
    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(pack, false);

    return m_data == pack->m_data;
}

MyPaintBrushInput sensorIdToMyPaintBrushInput(const KoID &id)
{
    MyPaintBrushInput result = MYPAINT_BRUSH_INPUT_PRESSURE;

    if (id == MyPaintPressureId) {
        result = MYPAINT_BRUSH_INPUT_PRESSURE;
    } else if (id == MyPaintFineSpeedId) {
        result = MYPAINT_BRUSH_INPUT_SPEED1;
    } else if (id == MyPaintGrossSpeedId) {
        result = MYPAINT_BRUSH_INPUT_SPEED2;
    } else if (id == MyPaintRandomId) {
        result = MYPAINT_BRUSH_INPUT_RANDOM;
    } else if (id == MyPaintStrokeId) {
        result = MYPAINT_BRUSH_INPUT_STROKE;
    } else if (id == MyPaintDirectionId) {
        result = MYPAINT_BRUSH_INPUT_DIRECTION;
    } else if (id == MyPaintDeclinationId) {
        result = MYPAINT_BRUSH_INPUT_TILT_DECLINATION;
    } else if (id == MyPaintAscensionId) {
        result = MYPAINT_BRUSH_INPUT_TILT_ASCENSION;
    } else if (id == MyPaintCustomId) {
        result = MYPAINT_BRUSH_INPUT_CUSTOM;
    } else {
        qWarning() << "sensorIdToMyPaintBrushInput: unknown sensor id";
    }

    return result;
}

QString sensorIdToMyPaintBrushInputJsonKey(const KoID &id)
{
    QString result = "pressure";

    if (id == MyPaintPressureId) {
        result = "pressure";
    } else if (id == MyPaintFineSpeedId) {
        result = "speed1";
    } else if (id == MyPaintGrossSpeedId) {
        result = "speed2";
    } else if (id == MyPaintRandomId) {
        result = "random";
    } else if (id == MyPaintStrokeId) {
        result = "stroke";
    } else if (id == MyPaintDirectionId) {
        result = "direction";
    } else if (id == MyPaintDeclinationId) {
        result = "tilt_declination";
    } else if (id == MyPaintAscensionId) {
        result = "tilt_ascension";
    } else if (id == MyPaintCustomId) {
        result = "custom";
    } else {
        qWarning() << "sensorIdToMyPaintBrushInputJsonKey: unknown sensor id";
    }

    return result;
}

MyPaintBrushSetting optionIdToMyPaintBrushSettings(const KoID &id) {
    if (id.id() == "eraser")
        return MYPAINT_BRUSH_SETTING_ERASER;
    else if (id.id() == "opaque")
        return MYPAINT_BRUSH_SETTING_OPAQUE;
    else if (id.id() == "smudge")
        return MYPAINT_BRUSH_SETTING_SMUDGE;
    else if (id.id() == "color_h")
        return MYPAINT_BRUSH_SETTING_COLOR_H;
    else if (id.id() == "color_s")
        return MYPAINT_BRUSH_SETTING_COLOR_S;
    else if (id.id() == "color_v")
        return MYPAINT_BRUSH_SETTING_COLOR_V;
    else if (id.id() == "colorize")
        return MYPAINT_BRUSH_SETTING_COLORIZE;
    else if (id.id() == "hardness")
        return MYPAINT_BRUSH_SETTING_HARDNESS;
    else if (id.id() == "speed1_gamma")
        return MYPAINT_BRUSH_SETTING_SPEED1_GAMMA;
    else if (id.id() == "speed2_gamma")
        return MYPAINT_BRUSH_SETTING_SPEED2_GAMMA;
    else if (id.id() == "anti_aliasing")
        return MYPAINT_BRUSH_SETTING_ANTI_ALIASING;
    else if (id.id() == "restore_color")
        return MYPAINT_BRUSH_SETTING_RESTORE_COLOR;
    else if (id.id() == "slow_tracking")
        return MYPAINT_BRUSH_SETTING_SLOW_TRACKING;
    else if (id.id() == "smudge_length")
        return MYPAINT_BRUSH_SETTING_SMUDGE_LENGTH;
    else if (id.id() == "snap_to_pixel")
        return MYPAINT_BRUSH_SETTING_SNAP_TO_PIXEL;
    else if (id.id() == "change_color_h")
        return MYPAINT_BRUSH_SETTING_CHANGE_COLOR_H;
    else if (id.id() == "change_color_l")
        return MYPAINT_BRUSH_SETTING_CHANGE_COLOR_L;
    else if (id.id() == "change_color_v")
        return MYPAINT_BRUSH_SETTING_CHANGE_COLOR_V;
    else if (id.id() == "tracking_noise")
        return MYPAINT_BRUSH_SETTING_TRACKING_NOISE;
    else if (id.id() == "dabs_per_second")
        return MYPAINT_BRUSH_SETTING_DABS_PER_SECOND;
    else if (id.id() == "offset_by_speed")
        return MYPAINT_BRUSH_SETTING_OFFSET_BY_SPEED;
    else if (id.id() == "opaque_multiply")
        return MYPAINT_BRUSH_SETTING_OPAQUE_MULTIPLY;
    else if (id.id() == "speed1_slowness")
        return MYPAINT_BRUSH_SETTING_SPEED1_SLOWNESS;
    else if (id.id() == "speed2_slowness")
        return MYPAINT_BRUSH_SETTING_SPEED2_SLOWNESS;
    else if (id.id() == "stroke_holdtime")
        return MYPAINT_BRUSH_SETTING_STROKE_HOLDTIME;
    else if (id.id() == "direction_filter")
        return MYPAINT_BRUSH_SETTING_DIRECTION_FILTER;
    else if (id.id() == "offset_by_random")
        return MYPAINT_BRUSH_SETTING_OFFSET_BY_RANDOM;
    else if (id.id() == "opaque_linearize")
        return MYPAINT_BRUSH_SETTING_OPAQUE_LINEARIZE;
    else if (id.id() == "radius_by_random")
        return MYPAINT_BRUSH_SETTING_RADIUS_BY_RANDOM;
    else if (id.id() == "stroke_threshold")
        return MYPAINT_BRUSH_SETTING_STROKE_THRESHOLD;
    else if (id.id() == "pressure_gain_log")
        return MYPAINT_BRUSH_SETTING_PRESSURE_GAIN_LOG;
    else if (id.id() == "smudge_radius_log")
        return MYPAINT_BRUSH_SETTING_SMUDGE_RADIUS_LOG;
    else if (id.id() == "change_color_hsl_s")
        return MYPAINT_BRUSH_SETTING_CHANGE_COLOR_HSL_S;
    else if (id.id() == "change_color_hsv_s")
        return MYPAINT_BRUSH_SETTING_CHANGE_COLOR_HSV_S;
    else if (id.id() == "radius_logarithmic")
        return MYPAINT_BRUSH_SETTING_RADIUS_LOGARITHMIC;
    else if (id.id() == "elliptical_dab_angle")
        return MYPAINT_BRUSH_SETTING_ELLIPTICAL_DAB_ANGLE;
    else if (id.id() == "elliptical_dab_ratio")
        return MYPAINT_BRUSH_SETTING_ELLIPTICAL_DAB_RATIO;
    else if (id.id() == "custom_input_slowness")
        return MYPAINT_BRUSH_SETTING_CUSTOM_INPUT_SLOWNESS;
    else if (id.id() == "custom_input")
        return MYPAINT_BRUSH_SETTING_CUSTOM_INPUT;
    else if (id.id() == "dabs_per_basic_radius")
        return MYPAINT_BRUSH_SETTING_DABS_PER_BASIC_RADIUS;
    else if (id.id() == "slow_tracking_per_dab")
        return MYPAINT_BRUSH_SETTING_SLOW_TRACKING_PER_DAB;
    else if (id.id() == "dabs_per_actual_radius")
        return MYPAINT_BRUSH_SETTING_DABS_PER_ACTUAL_RADIUS;
    else if (id.id() == "offset_by_speed_slowness")
        return MYPAINT_BRUSH_SETTING_OFFSET_BY_SPEED_SLOWNESS;
    else if (id.id() == "stroke_duration_logarithmic")
        return MYPAINT_BRUSH_SETTING_STROKE_DURATION_LOGARITHMIC;
    else if (id.id() == "smudge_length_log")
        return MYPAINT_BRUSH_SETTING_SMUDGE_LENGTH_LOG;
    else if (id.id() == "smudge_bucket")
        return MYPAINT_BRUSH_SETTING_SMUDGE_BUCKET;
    else if (id.id() == "smudge_transparency")
        return MYPAINT_BRUSH_SETTING_SMUDGE_TRANSPARENCY;
    else if (id.id() == "posterize")
        return MYPAINT_BRUSH_SETTING_POSTERIZE;
    else if (id.id() == "posterize_num")
        return MYPAINT_BRUSH_SETTING_POSTERIZE_NUM;

    return MYPAINT_BRUSH_SETTING_ERASER;
}

bool MyPaintSensorPack::read(KisCurveOptionDataCommon &data, const KisPropertiesConfiguration *setting) const
{
    data.isChecked = !data.isCheckable || setting->getBool("Pressure" + data.id.id(), false);

    std::vector<KisSensorData *> sensors = data.sensors();

    // TODO: refactor into library resource
    std::unique_ptr<MyPaintBrush, decltype(&mypaint_brush_unref)>
        brush(mypaint_brush_new(), &mypaint_brush_unref);

    const MyPaintBrushSetting brushSetting = optionIdToMyPaintBrushSettings(data.id);
    mypaint_brush_from_string(brush.get(), setting->getProperty(MYPAINT_JSON).toByteArray());

    for (auto it = sensors.begin(); it != sensors.end(); ++it) {
        const MyPaintBrushInput input = sensorIdToMyPaintBrushInput((*it)->id);
        const int numPoints = mypaint_brush_get_mapping_n(brush.get(), brushSetting, input);

        QList<QPointF> points;
        for(int i = 0; i < numPoints; i++) {
            float x, y;
            mypaint_brush_get_mapping_point(brush.get(), brushSetting, input, i, &x, &y);
            points << QPointF(qreal(x), qreal(y));
        }

        if (points.size() > 1) {
            (*it)->isActive = true;
            (*it)->curve = KisCubicCurve(points).toString();
            (*it)->setBaseCurveRange(KisAlgebra2D::accumulateBounds(points));
            dynamic_cast<MyPaintSensorDataWithRange*>(*it)->reshapeCurve();
        } else {
            (*it)->reset();
        }
    }

    // At least one sensor needs to be active
    if (std::find_if(sensors.begin(), sensors.end(),
                     std::mem_fn(&KisSensorData::isActive)) == sensors.end()) {

        sensors.front()->isActive = true;
        data.useCurve = false;
    } else {
        data.useCurve = true;
    }

    data.strengthValue = qreal(mypaint_brush_get_base_value(brush.get(), brushSetting));

    data.useSameCurve = false; // not used in mypaint engine
    data.commonCurve = DEFAULT_CURVE_STRING; // not used in mypaint engine
    data.curveMode = 0; // not used in mypaint engine

    return true;
}

void MyPaintSensorPack::write(const KisCurveOptionDataCommon &data, KisPropertiesConfiguration *setting) const
{
    setting->setProperty("Pressure" + data.id.id(), data.isChecked || !data.isCheckable);

    QVector<const KisSensorData*> activeSensors;
    Q_FOREACH(const KisSensorData *sensor, data.sensors()) {
        if (sensor->isActive) {
            activeSensors.append(sensor);
        }
    }

    QJsonDocument doc = QJsonDocument::fromJson(setting->getProperty(MYPAINT_JSON).toByteArray());

    QJsonObject brush_json = doc.object();
    QVariantMap map = brush_json.toVariantMap();
    QVariantMap settings_map = map["settings"].toMap();
    QVariantMap name_map = settings_map[data.id.id()].toMap();
    QVariantMap inputs_map = name_map["inputs"].toMap();

    const std::vector<const KisSensorData*> sensors = data.sensors();

    for (auto it = sensors.begin(); it != sensors.end(); ++it) {
        const KisSensorData *sensor = *it;
        const QString sensorJsonId = sensorIdToMyPaintBrushInputJsonKey(sensor->id);

        if (!sensor->isActive || !data.useCurve) {
            inputs_map.remove(sensorJsonId);
        } else {
            KisCubicCurve curve(sensor->curve);
            const QList<QPointF> &points = curve.points();

            QVariantList pointsList;

            Q_FOREACH(const QPointF &pt, points) {
                pointsList.push_back(QVariantList{pt.x(), pt.y()});
            }
            inputs_map[sensorJsonId] = pointsList;
        }
    }

    name_map["inputs"] = inputs_map;
    name_map["base_value"] = data.strengthValue;

    settings_map[data.id.id()] = name_map;
    map["settings"] = settings_map;

    doc = QJsonDocument(QJsonObject::fromVariantMap(map));
    setting->setProperty(MYPAINT_JSON, doc.toJson());

    // not used in mypaint engine
    setting->setProperty(data.id.id() + "UseSameCurve", false);
    setting->setProperty(data.id.id() + "commonCurve", DEFAULT_CURVE_STRING);
    setting->setProperty(data.id.id() + "curveMode", 0);
}
