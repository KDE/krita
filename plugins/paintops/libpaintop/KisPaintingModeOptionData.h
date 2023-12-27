/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISPAINTINGMODEOPTIONDATA_H
#define KISPAINTINGMODEOPTIONDATA_H


#include "kis_types.h"
#include <boost/operators.hpp>
#include <kritapaintop_export.h>

class KisPropertiesConfiguration;

enum class enumPaintingMode {
    BUILDUP,
    WASH
};

struct PAINTOP_EXPORT KisPaintingModeOptionData : boost::equality_comparable<KisPaintingModeOptionData>
{
    inline friend bool operator==(const KisPaintingModeOptionData &lhs, const KisPaintingModeOptionData &rhs) {
        return lhs.paintingMode == rhs.paintingMode;
    }

    enumPaintingMode paintingMode { enumPaintingMode::BUILDUP };
    bool hasPaintingModeProperty {false};

    bool read(const KisPropertiesConfiguration *setting);
    void write(KisPropertiesConfiguration *setting) const;
};

#endif // KISPAINTINGMODEOPTIONDATA_H
