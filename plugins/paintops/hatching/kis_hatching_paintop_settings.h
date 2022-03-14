/*
 *  SPDX-FileCopyrightText: 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *  SPDX-FileCopyrightText: 2010 José Luis Vergara <pentalis@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_HATCHING_PAINTOP_SETTINGS_H_
#define KIS_HATCHING_PAINTOP_SETTINGS_H_

#include <brushengine/kis_paintop_settings.h>
#include <kis_brush_based_paintop_settings.h>

#include "kis_hatching_paintop_settings_widget.h"

#include <QScopedPointer>


class KisHatchingPaintOpSettings : public KisBrushBasedPaintOpSettings
{

public:
    KisHatchingPaintOpSettings(KisResourcesInterfaceSP resourcesInterface);
    ~KisHatchingPaintOpSettings() override;

    //Dialogs enabled
    bool enabledcurveangle {false};
    bool enabledcurvecrosshatching {false};
    bool enabledcurveopacity {false};
    bool enabledcurveseparation {false};
    bool enabledcurvesize {false};
    bool enabledcurvethickness {false};

    //Hatching Options
    double angle {0.0};
    double separation {0.0};
    double thickness {0.0};
    double origin_x {0.0};
    double origin_y {0.0};
    bool nocrosshatching {false};
    bool perpendicular {false};
    bool minusthenplus {false};
    bool plusthenminus {false};
    bool moirepattern {false};
    int crosshatchingstyle {0};
    int separationintervals {0};

    //Hatching Preferences
    //bool trigonometryalgebra {false};
    //bool scratchoff {false};
    bool antialias {false};
    bool subpixelprecision {false};
    bool opaquebackground {false};

    //Angle, Crosshatching, Separation and Thickness curves
    double anglesensorvalue {0.0};
    double crosshatchingsensorvalue {0.0};
    double separationsensorvalue {0.0};
    double thicknesssensorvalue {0.0};

    void initializeTwin(KisPaintOpSettingsSP convenienttwin) const;
    using KisPropertiesConfiguration::fromXML;
    void fromXML(const QDomElement&) override;

    QList<KisUniformPaintOpPropertySP> uniformProperties(KisPaintOpSettingsSP settings, QPointer<KisPaintOpPresetUpdateProxy> updateProxy) override;

private:
    Q_DISABLE_COPY(KisHatchingPaintOpSettings)

    struct Private;
    const QScopedPointer<Private> m_d;

};

typedef KisSharedPtr<KisHatchingPaintOpSettings> KisHatchingPaintOpSettingsSP;

#endif
