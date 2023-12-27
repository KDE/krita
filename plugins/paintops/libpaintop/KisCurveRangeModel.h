/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISCURVERANGEMODEL_H
#define KISCURVERANGEMODEL_H

#include <KisCurveRangeModelInterface.h>
#include <lager/constant.hpp>

class PAINTOP_EXPORT KisCurveRangeModel : public KisCurveRangeModelInterface
{
public:
    KisCurveRangeModel(lager::cursor<QString> curve,
                       lager::reader<QString> activeSensorId,
                       lager::reader<int> activeSensorLength,
                       const QString &yMinLabel,
                       const QString &yMaxLabel,
                       int yMinValue,
                       int yMaxValue,
                       const QString &yValueSuffix);

    ~KisCurveRangeModel();

    lager::cursor<QString> curve() override;
    lager::reader<QString> xMinLabel() override;
    lager::reader<QString> xMaxLabel() override;
    lager::reader<QString> yMinLabel() override;
    lager::reader<QString> yMaxLabel() override;
    lager::reader<qreal> yMinValue() override;
    lager::reader<qreal> yMaxValue() override;
    lager::reader<QString> yValueSuffix() override;
    lager::reader<qreal> xMinValue() override;
    lager::reader<qreal> xMaxValue() override;
    lager::reader<QString> xValueSuffix() override;

    static KisCurveRangeModelFactory factory(const QString &yMinLabel, const QString &yMaxLabel, int curveMinValue, int curveMaxValue, const QString &curveValueSuffix);

    static qreal calcXMinValueWithFactory(const QString &sensorId);
    static qreal calcXMaxValueWithFactory(const QString &activeSensorId, const int length);

private:
    lager::cursor<QString> m_curve;
    lager::reader<QString> m_activeSensorId;
    lager::reader<int> m_activeSensorLength;
    lager::constant<QString> m_yMinLabel;
    lager::constant<QString> m_yMaxLabel;
    lager::constant<qreal> m_yMinValue;
    lager::constant<qreal> m_yMaxValue;
    lager::constant<QString> m_yValueSuffix;
};

#endif // KISCURVERANGEMODEL_H
