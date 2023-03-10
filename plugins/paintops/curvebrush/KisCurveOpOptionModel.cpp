/*
 *  SPDX-FileCopyrightText: 2022 Agata Cacko <cacko.azh@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisCurveOpOptionModel.h"

#include <KisLager.h>


KisCurveOpOptionModel::KisCurveOpOptionModel(lager::cursor<KisCurveOpOptionData> _optionData)
    : optionData(_optionData)
    , LAGER_QT(curvePaintConnectionLine) {_optionData[&KisCurveOpOptionData::curve_paint_connection_line]}
    , LAGER_QT(curveSmoothing) {_optionData[&KisCurveOpOptionData::curve_smoothing]}
    , LAGER_QT(curveStrokeHistorySize) {_optionData[&KisCurveOpOptionData::curve_stroke_history_size].zoom(kislager::lenses::do_static_cast<int, qreal>)}
    , LAGER_QT(curveLineWidth) {_optionData[&KisCurveOpOptionData::curve_line_width].zoom(kislager::lenses::do_static_cast<int, qreal>)}
    , LAGER_QT(curveCurvesOpacity) {_optionData[&KisCurveOpOptionData::curve_curves_opacity]}
{
}
