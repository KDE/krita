/*
 *  SPDX-FileCopyrightText: 2014 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_BSPLINE_H
#define __KIS_BSPLINE_H

namespace KisBSplines {

    enum BorderCondition {
        Periodic,
        Deriv1,
        Deriv2,
        Flat,
        Natural,
        Antiperiodic
    };

}

#endif /* __KIS_BSPLINE_H */
