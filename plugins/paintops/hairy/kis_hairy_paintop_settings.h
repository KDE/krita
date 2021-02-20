/*
 *  SPDX-FileCopyrightText: 2008 Boudewijn Rempt <boud@valdyas.org>
 *  SPDX-FileCopyrightText: 2008-2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_HAIRYPAINTOP_SETTINGS_H_
#define KIS_HAIRYPAINTOP_SETTINGS_H_

#include <QList>

#include <kis_brush_based_paintop_settings.h>
#include <kis_types.h>


class KisHairyPaintOpSettings : public KisBrushBasedPaintOpSettings
{

public:
    using KisPaintOpSettings::fromXML;

    KisHairyPaintOpSettings(KisResourcesInterfaceSP resourcesInterface);
    using KisBrushBasedPaintOpSettings::brushOutline;
    QPainterPath brushOutline(const KisPaintInformation &info, const OutlineMode &mode, qreal alignForZoom) override;
    bool hasPatternSettings() const override;

};

#endif
