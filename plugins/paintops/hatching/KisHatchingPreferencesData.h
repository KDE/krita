/*
 *  SPDX-FileCopyrightText: 2008 Lukas Tvrdy <lukast.dev@gmail.com>
 *  SPDX-FileCopyrightText: 2010 José Luis Vergara <pentalis@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KIS_HATCHING_PREFERENCES_DATA_H
#define KIS_HATCHING_PREFERENCES_DATA_H


#include "kis_types.h"
#include <boost/operators.hpp>

class KisPropertiesConfiguration;

struct KisHatchingPreferencesData : boost::equality_comparable<KisHatchingPreferencesData>
{
    inline friend bool operator==(const KisHatchingPreferencesData &lhs, const KisHatchingPreferencesData &rhs) {
        return lhs.useAntialias == rhs.useAntialias
            && lhs.useOpaqueBackground == rhs.useOpaqueBackground
            && lhs.useSubpixelPrecision == rhs.useSubpixelPrecision;
    }

    bool useAntialias {false};
    bool useOpaqueBackground {false};
    bool useSubpixelPrecision {false};

    bool read(const KisPropertiesConfiguration *setting);
    void write(KisPropertiesConfiguration *setting) const;
};

#endif // KIS_HATCHING_PREFERENCES_DATA_H
