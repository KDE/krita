/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef MYPAINTCURVERANGEMODEL_H
#define MYPAINTCURVERANGEMODEL_H

#include <QObject>
#include <KisCurveRangeModelInterface.h>
#include <lager/extra/qt.hpp>
#include <KisWidgetConnectionUtils.h>


class MyPaintCurveRangeModel : public QObject, public KisCurveRangeModelInterface
{
    Q_OBJECT
public:
    struct NormalizedCurve {
        QString curve;
        qreal xMin = 0.0;
        qreal xMax = 1.0;
        qreal yLimit = 1.0;
    };

public:
    MyPaintCurveRangeModel(lager::cursor<QString> curve,
                           lager::cursor<QRectF> curveRange,
                           lager::reader<QString> activeSensorId,
                           lager::reader<int> activeSensorLength,
                           qreal maxYRange, const QString &yValueSuffix);
    ~MyPaintCurveRangeModel();

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

    qreal maxYRange() const;

    static KisCurveRangeModelFactory factory(qreal maxYRange, const QString &yValueSuffix);

    static std::tuple<QString, QRectF> reshapeCurve(std::tuple<QString, QRectF> curve);

private:
    lager::cursor<QString> m_curve;
    lager::cursor<QRectF> m_curveRange;
    lager::reader<QString> m_activeSensorId;
    lager::reader<int> m_activeSensorLength;
    lager::cursor<NormalizedCurve> m_normalizedCurve;
    const qreal m_maxYRange {0.0};
    const QString m_yValueSuffix;

public:
    LAGER_QT_CURSOR(qreal, yLimit);
    LAGER_QT_CURSOR(qreal, xMin);
    LAGER_QT_CURSOR(qreal, xMax);
    LAGER_QT_READER(DoubleSpinBoxState, xMinState);
    LAGER_QT_READER(DoubleSpinBoxState, xMaxState);
};

#endif // MYPAINTCURVERANGEMODEL_H
