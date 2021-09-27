/*
 *  SPDX-FileCopyrightText: 2017 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISMASKINGBRUSHOPTIONPROPERTIES_H
#define KISMASKINGBRUSHOPTIONPROPERTIES_H

#include "kritapaintop_export.h"
#include <kis_types.h>
#include <kis_brush.h>

class KisResourcesInterface;
using KisResourcesInterfaceSP = QSharedPointer<KisResourcesInterface>;

class KoCanvasResourcesInterface;
using KoCanvasResourcesInterfaceSP = QSharedPointer<KoCanvasResourcesInterface>;


struct PAINTOP_EXPORT KisMaskingBrushOptionProperties
{
    KisMaskingBrushOptionProperties();

    bool isEnabled = false;
    KisBrushSP brush;
    QString compositeOpId;
    bool useMasterSize = true;

    boost::optional<qreal> theoreticalMaskingBrushSize;

    void write(KisPropertiesConfiguration *setting, qreal masterBrushSize) const;
    void read(const KisPropertiesConfiguration *setting, qreal masterBrushSize, KisResourcesInterfaceSP resourcesInterface, KoCanvasResourcesInterfaceSP canvasResourcesInterface);
    QList<KoResourceSP> prepareLinkedResources(const KisPropertiesConfigurationSP settings, KisResourcesInterfaceSP resourcesInterface);
};

#endif // KISMASKINGBRUSHOPTIONPROPERTIES_H
