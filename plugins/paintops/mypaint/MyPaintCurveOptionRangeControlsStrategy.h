/*
 * SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef MYPAINTCURVEOPTIONRANGECONTROLSSTRATEGY_H
#define MYPAINTCURVEOPTIONRANGECONTROLSSTRATEGY_H

#include <KisCurveOptionRangeControlsStrategyInterface.h>
#include <lager/reader.hpp>
#include <QString>

class MyPaintCurveRangeModel;

class MyPaintCurveOptionRangeControlsStrategy : public KisCurveOptionRangeControlsStrategyInterface
{
public:
    MyPaintCurveOptionRangeControlsStrategy(KisCurveRangeModelInterface* rangeInterface,
                                            QWidget* rangeControlsPlaceholder);
    ~MyPaintCurveOptionRangeControlsStrategy();

    static KisCurveOptionRangeControlsStrategyFactory factory();

private:
    MyPaintCurveRangeModel *m_rangeModel;
    lager::reader<QString> m_xValueSuffix;
    lager::reader<QString> m_yValueSuffix;
};

#endif // MYPAINTCURVEOPTIONRANGECONTROLSSTRATEGY_H
