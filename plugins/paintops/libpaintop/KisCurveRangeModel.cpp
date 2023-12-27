/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisCurveRangeModel.h"

#include <KisDynamicSensorFactoryRegistry.h>

using LabelsState = std::tuple<QString, int>;

namespace {
QString calcMinLabelWithFactory(const QString &sensorId) 
{
    KisDynamicSensorFactory *factory =
            KisDynamicSensorFactoryRegistry::instance()->get(sensorId);

    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(factory, "");

    return factory->minimumLabel();
}

QString calcMaxLabelWithFactory(const QString &activeSensorId, const int length) 
{
    KisDynamicSensorFactory *factory =
            KisDynamicSensorFactoryRegistry::instance()->get(activeSensorId);

    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(factory, "");

    return factory->maximumLabel(length);
}

QString calcValueSuffixWithFactory(const QString &activeSensorId)
{
    KisDynamicSensorFactory *factory =
            KisDynamicSensorFactoryRegistry::instance()->get(activeSensorId);

    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(factory, "");

    return factory->valueSuffix();
}


}

KisCurveRangeModel::KisCurveRangeModel(lager::cursor<QString> curve,
                                       lager::reader<QString> activeSensorId,
                                       lager::reader<int> activeSensorLength,
                                       const QString &yMinLabel,
                                       const QString &yMaxLabel,
                                       int yMinValue,
                                       int yMaxValue,
                                       const QString &yValueSuffix)
    : m_curve(std::move(curve))
    , m_activeSensorId(std::move(activeSensorId))
    , m_activeSensorLength(std::move(activeSensorLength))
    , m_yMinLabel(yMinLabel)
    , m_yMaxLabel(yMaxLabel)
    , m_yMinValue(yMinValue)
    , m_yMaxValue(yMaxValue)
    , m_yValueSuffix(yValueSuffix)
{
}

KisCurveRangeModelFactory KisCurveRangeModel::factory(const QString &yMinLabel,
                                                      const QString &yMaxLabel,
                                                      int curveMinValue,
                                                      int curveMaxValue,
                                                      const QString &curveValueSuffix)
{
    return [yMinLabel, yMaxLabel,
            curveMinValue, curveMaxValue,
            curveValueSuffix](lager::cursor<QString> curve,
                              lager::cursor<QRectF> curveRange,
                              lager::reader<QString> activeSensorId,
                              lager::reader<int> activeSensorLength) {

        Q_UNUSED(curveRange);
        return new KisCurveRangeModel(curve, activeSensorId, activeSensorLength, yMinLabel, yMaxLabel, curveMinValue, curveMaxValue, curveValueSuffix);
    };
}

qreal KisCurveRangeModel::calcXMinValueWithFactory(const QString &sensorId)
{
    KisDynamicSensorFactory *factory =
        KisDynamicSensorFactoryRegistry::instance()->get(sensorId);

    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(factory, 0.0);

    return qreal(factory->minimumValue());
}

qreal KisCurveRangeModel::calcXMaxValueWithFactory(const QString &activeSensorId, const int length)
{
    KisDynamicSensorFactory *factory =
        KisDynamicSensorFactoryRegistry::instance()->get(activeSensorId);

    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(factory, 1.0);

    return factory->maximumValue(length);
}

KisCurveRangeModel::~KisCurveRangeModel()
{
}

lager::cursor<QString> KisCurveRangeModel::curve()
{
    return m_curve;
}

lager::reader<QString> KisCurveRangeModel::xMinLabel()
{
    return m_activeSensorId.map(&calcMinLabelWithFactory);
}

lager::reader<QString> KisCurveRangeModel::xMaxLabel()
{
    return lager::with(m_activeSensorId, m_activeSensorLength).map(&calcMaxLabelWithFactory);
}

lager::reader<QString> KisCurveRangeModel::yMinLabel()
{
    return m_yMinLabel;
}

lager::reader<QString> KisCurveRangeModel::yMaxLabel()
{
    return m_yMaxLabel;
}

lager::reader<qreal> KisCurveRangeModel::yMinValue()
{
    return m_yMinValue;
}

lager::reader<qreal> KisCurveRangeModel::yMaxValue()
{
    return m_yMaxValue;
}

lager::reader<QString> KisCurveRangeModel::yValueSuffix()
{
    return m_yValueSuffix;
}

lager::reader<qreal> KisCurveRangeModel::xMinValue()
{
    return m_activeSensorId.map(&calcXMinValueWithFactory);
}

lager::reader<qreal> KisCurveRangeModel::xMaxValue()
{
    return lager::with(m_activeSensorId, m_activeSensorLength).map(&calcXMaxValueWithFactory);
}

lager::reader<QString> KisCurveRangeModel::xValueSuffix()
{
    return m_activeSensorId.map(&calcValueSuffixWithFactory);
}
