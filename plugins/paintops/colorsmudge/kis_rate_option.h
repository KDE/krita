/* This file is part of the KDE project
 *
 * SPDX-FileCopyrightText: 2011 Silvio Heinrich <plassy@web.de>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KIS_RATE_OPTION_H
#define KIS_RATE_OPTION_H

#include "kis_curve_option.h"
#include <brushengine/kis_paint_information.h>
#include <kis_types.h>

class KisPainter;

class KisRateOption: public KisCurveOption
{
public:
    KisRateOption(const KoID &name, KisPaintOpOption::PaintopCategory category, bool checked);

    /**
     * Set the opacity of the painter based on the rate
     * and the curve (if checked)
     */
    void apply(KisPainter& painter, const KisPaintInformation& info, qreal scaleMin = 0.0, qreal scaleMax = 1.0, qreal multiplicator = 1.0) const;

    void setRate(qreal rate) {
        KisCurveOption::setValue(rate);
    }
    qreal getRate() const {
        return KisCurveOption::value();
    }
};

#endif // KIS_RATE_OPTION_H
