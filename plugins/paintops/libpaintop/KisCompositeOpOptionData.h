/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISCOMPOSITEOPOPTIONDATA_H
#define KISCOMPOSITEOPOPTIONDATA_H

#include "kis_types.h"
#include <boost/operators.hpp>
#include <kritapaintop_export.h>

class KisPropertiesConfiguration;

struct PAINTOP_EXPORT KisCompositeOpOptionData : boost::equality_comparable<KisCompositeOpOptionData>
{
    KisCompositeOpOptionData();

    inline friend bool operator==(const KisCompositeOpOptionData &lhs, const KisCompositeOpOptionData &rhs) {
        return lhs.compositeOpId == rhs.compositeOpId &&
                lhs.eraserMode == rhs.eraserMode;
    }

    QString compositeOpId;
    bool eraserMode {false};

    bool read(const KisPropertiesConfiguration *setting);
    void write(KisPropertiesConfiguration *setting) const;
};

#endif // KISCOMPOSITEOPOPTIONDATA_H
