/*
 *  SPDX-FileCopyrightText: 2017 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISMASKINGBRUSHOPTIONPROPERTIES_H
#define KISMASKINGBRUSHOPTIONPROPERTIES_H

#include "kritapaintop_export.h"
#include <kis_types.h>
#include <KisBrushModel.h>
#include <KoCompositeOpRegistry.h>

class KisResourcesInterface;
using KisResourcesInterfaceSP = QSharedPointer<KisResourcesInterface>;

class KoCanvasResourcesInterface;
using KoCanvasResourcesInterfaceSP = QSharedPointer<KoCanvasResourcesInterface>;

namespace KisBrushModel {
struct PAINTOP_EXPORT MaskingBrushData :  public boost::equality_comparable<MaskingBrushData>
{
    bool isEnabled = false;
    BrushData brush;
    QString compositeOpId = COMPOSITE_MULT;
    bool useMasterSize = true;
    qreal masterSizeCoeff = 1.0;

    friend bool operator==(const MaskingBrushData &lhs, const MaskingBrushData &rhs);
    static MaskingBrushData read(const KisPropertiesConfiguration *config, qreal masterBrushSize, KisResourcesInterfaceSP resourcesInterface);
    void write(KisPropertiesConfiguration *config) const;
};

bool operator==(const MaskingBrushData &lhs, const MaskingBrushData &rhs);

}

#endif // KISMASKINGBRUSHOPTIONPROPERTIES_H
