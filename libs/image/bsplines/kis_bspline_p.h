/*
 *  SPDX-FileCopyrightText: 2014 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_BSPLINE_P_H
#define __KIS_BSPLINE_P_H

#include "einspline/bspline_base.h"

namespace KisBSplines {

    inline bc_code convertBorderType(BorderCondition v) {
        switch (v) {
        case Periodic:
            return PERIODIC;
        case Deriv1:
            return DERIV1;
        case Deriv2:
            return DERIV2;
        case Flat:
            return FLAT;
        case Natural:
            return NATURAL;
        case Antiperiodic:
            return ANTIPERIODIC;
        }

        return NATURAL;
    }

}

#endif /* __KIS_BSPLINE_P_H */
