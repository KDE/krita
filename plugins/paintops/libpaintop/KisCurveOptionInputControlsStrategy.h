/*
 * SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISCURVEOPTIONINPUTCONTROLSSTRATEGY_H
#define KISCURVEOPTIONINPUTCONTROLSSTRATEGY_H

#include <kritapaintop_export_instance.h>

#include <lager/reader.hpp>
#include <QScopedPointer>
#include <KisCurveOptionInputControlsStrategyInterface.h>
#include <QString>

class KisCurveWidgetControlsManagerBase;
class QSpinBox;
class QDoubleSpinBox;

template <typename SpinBox>
class PAINTOP_EXPORT_TEMPLATE KisCurveOptionInputControlsStrategy
    : public KisCurveOptionInputControlsStrategyInterface
{
public:
    KisCurveOptionInputControlsStrategy(KisCurveRangeModelInterface *rangeInterface,
                                        KisCurveWidget *curveWidget,
                                        QWidget *inPlaceholder, QWidget *outPlaceholder);
    ~KisCurveOptionInputControlsStrategy();

    static KisCurveOptionInputControlsStrategyFactory factory();

private:
    void updateCurveLabels(qreal xMin, qreal xMax, qreal yMin, qreal yMax);

private:
    SpinBox *inSpinBox;
    SpinBox *outSpinBox;
    KisCurveWidget *curveWidget {nullptr};
    QScopedPointer<KisCurveWidgetControlsManagerBase> curveControlsManager;
    lager::reader<QString> xValueSuffix;
    lager::reader<QString> yValueSuffix;
    lager::reader<std::tuple<qreal, qreal, qreal, qreal>> rangeValues;
};

extern template class KisCurveOptionInputControlsStrategy<QSpinBox>;
extern template class KisCurveOptionInputControlsStrategy<QDoubleSpinBox>;

class KisCurveOptionInputControlsStrategyInt : public KisCurveOptionInputControlsStrategy<QSpinBox>
{
public:
    using KisCurveOptionInputControlsStrategy::KisCurveOptionInputControlsStrategy;
};

class KisCurveOptionInputControlsStrategyDouble : public KisCurveOptionInputControlsStrategy<QDoubleSpinBox>
{
public:
    using KisCurveOptionInputControlsStrategy::KisCurveOptionInputControlsStrategy;
};


#endif // KISCURVEOPTIONINPUTCONTROLSSTRATEGY_H
