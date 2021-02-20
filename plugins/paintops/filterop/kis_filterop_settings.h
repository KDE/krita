/*
 *  SPDX-FileCopyrightText: 2002 Patrick Julien <freak@codepimps.org>
 *  SPDX-FileCopyrightText: 2004-2008 Boudewijn Rempt <boud@valdyas.org>
 *  SPDX-FileCopyrightText: 2004 Clarence Dang <dang@kde.org>
 *  SPDX-FileCopyrightText: 2004 Adrian Page <adrian@pagenet.plus.com>
 *  SPDX-FileCopyrightText: 2004 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_FILTEROP_SETTINGS_H_
#define KIS_FILTEROP_SETTINGS_H_

#include <kis_brush_based_paintop_settings.h>
#include <kis_types.h>

#include "kis_filterop_settings_widget.h"

class QDomElement;
class KisFilterConfiguration;

class KisFilterOpSettings : public KisBrushBasedPaintOpSettings
{

public:
    KisFilterOpSettings(KisResourcesInterfaceSP resourcesInterface);

    ~KisFilterOpSettings() override;
    bool paintIncremental() override;

    KisFilterConfigurationSP filterConfig() const;

    using KisPaintOpSettings::toXML;
    void toXML(QDomDocument& doc, QDomElement& root) const override;

    using KisPaintOpSettings::fromXML;
    void fromXML(const QDomElement& e) override;

    bool hasPatternSettings() const override;
};


#endif // KIS_FILTEROP_SETTINGS_H_
