/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "MyPaintCurveRangeModel.h"
#include "kis_algebra_2d.h"
#include "kis_cubic_curve.h"
#include <lager/lenses.hpp>
#include <lager/constant.hpp>
#include <KisZug.h>
#include <KisDynamicSensorFactoryRegistry.h>
#include <KisCurveRangeModel.h>

namespace {
    auto formatQRealAsString = [] (qreal value) {
        return QString("%1").arg(value, 0, 'f', 2);
    };

    auto formatQRealAsStringWithSuffix = [] (const QString &suffix) {
        return [suffix] (qreal value) {
            return QString("%1%2").arg(value, 0, 'f', 2).arg(suffix);
        };
    };


    auto curveToNormalizedCurve = lager::lenses::getset(
        [] (const std::tuple<QString, QRectF> &curveData)
        {
            MyPaintCurveRangeModel::NormalizedCurve normalized;
            QList<QPointF> points = KisCubicCurve(std::get<0>(curveData)).points();
            const QRectF bounds = std::get<1>(curveData);

            normalized.yLimit = qMax(qAbs(bounds.top()), qAbs(bounds.bottom()));
            normalized.xMax = bounds.right();
            normalized.xMin = bounds.left();
            
            if (qFuzzyIsNull(normalized.yLimit)) {
                points = {{0.0,0.5}, {1.0, 0.5}};
            } else {
                for (auto it = points.begin(); it != points.end(); ++it) {
                    it->rx() = (it->x() - bounds.left()) / bounds.width();
                    it->ry() = it->y() / (2.0 * normalized.yLimit) + 0.5;
                }
            }
            
            normalized.curve = KisCubicCurve(points).toString();

            //qDebug() << "get" << std::get<0>(curveData) << "->" << normalized.curve << bounds;
            return normalized;
        },
        [] (std::tuple<QString, QRectF> curveData, const MyPaintCurveRangeModel::NormalizedCurve &normalizedCurve) {
            QList<QPointF> points = KisCubicCurve(normalizedCurve.curve).points();

            for (auto it = points.begin(); it != points.end(); ++it) {
                it->rx() = it->x() * (normalizedCurve.xMax - normalizedCurve.xMin) + normalizedCurve.xMin;
                it->ry() = (it->y() - 0.5) * normalizedCurve.yLimit * 2.0;
            }

            std::get<0>(curveData) = KisCubicCurve(points).toString();

            std::get<1>(curveData) = QRectF(normalizedCurve.xMin,
                                            -normalizedCurve.yLimit,
                                            normalizedCurve.xMax - normalizedCurve.xMin,
                                            2.0 * normalizedCurve.yLimit);

            //qDebug() << "set" << std::get<0>(curveData) << "<-" << normalizedCurve.curve << std::get<1>(curveData);
            return curveData;
        }
    );

} // namespace


using KisWidgetConnectionUtils::ToSpinBoxState;

MyPaintCurveRangeModel::MyPaintCurveRangeModel(lager::cursor<QString> curve,
                                               lager::cursor<QRectF> curveRange,
                                               lager::reader<QString> activeSensorId,
                                               lager::reader<int> activeSensorLength,
                                               qreal maxYRange,
                                               const QString &yValueSuffix)
    : m_curve(std::move(curve))
    , m_curveRange(std::move(curveRange))
    , m_activeSensorId(std::move(activeSensorId))
    , m_activeSensorLength(std::move(activeSensorLength))
    , m_normalizedCurve(lager::with(m_curve, m_curveRange).zoom(curveToNormalizedCurve))
    , m_maxYRange(maxYRange)
    , m_yValueSuffix(yValueSuffix)
    , LAGER_QT(yLimit) {m_normalizedCurve[&NormalizedCurve::yLimit]}
    , LAGER_QT(xMin) {m_normalizedCurve[&NormalizedCurve::xMin]}
    , LAGER_QT(xMax) {m_normalizedCurve[&NormalizedCurve::xMax]}
    , LAGER_QT(xMinState) {lager::with(m_normalizedCurve[&NormalizedCurve::xMin],
                                      m_activeSensorId.map(&KisCurveRangeModel::calcXMinValueWithFactory),
                                      lager::make_constant(0.0),
                                      lager::make_constant(true))
                              .map(ToSpinBoxState{})}
    , LAGER_QT(xMaxState) {lager::with(m_normalizedCurve[&NormalizedCurve::xMax],
                                      lager::make_constant(1.0),
                                      lager::with(m_activeSensorId, m_activeSensorLength)
                                          .map(&KisCurveRangeModel::calcXMaxValueWithFactory),
                                      lager::make_constant(true))
                              .map(ToSpinBoxState{})}
{

}

MyPaintCurveRangeModel::~MyPaintCurveRangeModel()
{

}

KisCurveRangeModelFactory MyPaintCurveRangeModel::factory(qreal maxYRange, const QString &yValueSuffix)
{
    return
        [maxYRange, yValueSuffix](lager::cursor<QString> curve, lager::cursor<QRectF> curveRange, lager::reader<QString> activeSensorId, lager::reader<int> activeSensorLength) {
            return new MyPaintCurveRangeModel(curve, curveRange, activeSensorId, activeSensorLength, maxYRange, yValueSuffix);
            };
}

std::tuple<QString, QRectF> MyPaintCurveRangeModel::reshapeCurve(std::tuple<QString, QRectF> curve)
{
    /**
     * Krita's GUI doesn't support x-range more narrow than 0...1, so
     * we should extend it if necessary
     */
    std::get<1>(curve) |= QRect(0, -1, 1, 2);

    NormalizedCurve normalized = lager::view(curveToNormalizedCurve, curve);
    curve = lager::set(curveToNormalizedCurve, curve, normalized);
    return curve;
}

lager::cursor<QString> MyPaintCurveRangeModel::curve()
{
    return m_normalizedCurve[&NormalizedCurve::curve];
}

lager::reader<QString> MyPaintCurveRangeModel::xMinLabel()
{
    return m_normalizedCurve[&NormalizedCurve::xMin].map(formatQRealAsString);
}

lager::reader<QString> MyPaintCurveRangeModel::xMaxLabel()
{
    return m_normalizedCurve[&NormalizedCurve::xMax].map(formatQRealAsString);
}

lager::reader<QString> MyPaintCurveRangeModel::yMinLabel()
{
    return yMinValue().map(formatQRealAsStringWithSuffix(m_yValueSuffix));
}

lager::reader<QString> MyPaintCurveRangeModel::yMaxLabel()
{
    return yMaxValue().map(formatQRealAsStringWithSuffix(m_yValueSuffix));
}

lager::reader<qreal> MyPaintCurveRangeModel::yMinValue()
{
    return m_normalizedCurve[&NormalizedCurve::yLimit].xform(kiszug::map_muptiply<qreal>(-1.0));
}

lager::reader<qreal> MyPaintCurveRangeModel::yMaxValue()
{
    return m_normalizedCurve[&NormalizedCurve::yLimit];
}

lager::reader<QString> MyPaintCurveRangeModel::yValueSuffix()
{
    return lager::make_constant(m_yValueSuffix);
}

lager::reader<qreal> MyPaintCurveRangeModel::xMinValue()
{
    return m_normalizedCurve[&NormalizedCurve::xMin];
}

lager::reader<qreal> MyPaintCurveRangeModel::xMaxValue()
{
    return m_normalizedCurve[&NormalizedCurve::xMax];
}

lager::reader<QString> MyPaintCurveRangeModel::xValueSuffix()
{
    return lager::make_constant(QString());
}

qreal MyPaintCurveRangeModel::maxYRange() const
{
    return m_maxYRange;
}
