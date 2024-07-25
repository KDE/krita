/*
 *  SPDX-FileCopyrightText: 2022 Agata Cacko <cacko.azh@gmail.com>
 *  SPDX-FileCopyrightText: 2011 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KIS_CURVEOP_OPTION_DATA_H
#define KIS_CURVEOP_OPTION_DATA_H


#include "kis_types.h"
#include <boost/operators.hpp>

class KisPropertiesConfiguration;


struct KisCurveOpOptionData : boost::equality_comparable<KisCurveOpOptionData>
{
    inline friend bool operator==(const KisCurveOpOptionData &lhs, const KisCurveOpOptionData &rhs) {
        return lhs.curve_paint_connection_line == rhs.curve_paint_connection_line
			&& lhs.curve_smoothing == rhs.curve_smoothing
			&& lhs.curve_stroke_history_size == rhs.curve_stroke_history_size
			&& lhs.curve_line_width == rhs.curve_line_width
			&& lhs.curve_curves_opacity == rhs.curve_curves_opacity;
    }


	bool curve_paint_connection_line {false};
    bool curve_smoothing {false};
    int curve_stroke_history_size {30};
    int curve_line_width {1};
    qreal curve_curves_opacity {1.0};

    bool read(const KisPropertiesConfiguration *setting);
    void write(KisPropertiesConfiguration *setting) const;
};

#endif // KIS_CURVEOP_OPTION_DATA_H
