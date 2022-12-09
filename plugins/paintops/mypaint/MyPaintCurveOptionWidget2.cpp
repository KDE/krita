/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "MyPaintCurveOptionWidget2.h"
#include "KisZug.h"
#include "kis_paintop_option.h"
#include "MyPaintCurveRangeModel.h"
#include "MyPaintCurveOptionRangeControlsStrategy.h"
#include "KisCurveOptionInputControlsStrategy.h"

MyPaintCurveOptionWidget2::MyPaintCurveOptionWidget2(lager::cursor<MyPaintCurveOptionData> optionData,
                                                     qreal maxYRange,
                                                     const QString &yValueSuffix)
    : KisCurveOptionWidget2(optionData.zoom(kiszug::lenses::to_base<KisCurveOptionDataCommon>),
                            KisPaintOpOption::GENERAL,
                            i18n("Base Value: "), yValueSuffix, 1.0,
                            lager::make_constant(true),
                            std::nullopt,
                            MyPaintCurveRangeModel::factory(maxYRange, yValueSuffix),
                            KisCurveOptionInputControlsStrategyDouble::factory(),
                            MyPaintCurveOptionRangeControlsStrategy::factory(),
                            None)
{
}

MyPaintCurveOptionWidget2::~MyPaintCurveOptionWidget2()
{
}
