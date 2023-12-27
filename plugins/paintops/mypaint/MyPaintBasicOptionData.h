/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef MYPAINTBASICOPTIONDATA_H
#define MYPAINTBASICOPTIONDATA_H

#include "kis_types.h"
#include <boost/operators.hpp>

class KisPropertiesConfiguration;

struct MyPaintBasicOptionData : boost::equality_comparable<MyPaintBasicOptionData>
{
    inline friend bool operator==(const MyPaintBasicOptionData &lhs, const MyPaintBasicOptionData &rhs) {
        return lhs.eraserMode == rhs.eraserMode;
    }

    bool eraserMode {false};

    bool read(const KisPropertiesConfiguration *setting);
    void write(KisPropertiesConfiguration *setting) const;
};

#endif // MYPAINTBASICOPTIONDATA_H
