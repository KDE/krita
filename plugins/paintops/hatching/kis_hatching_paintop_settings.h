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
    bool enabledcurveangle;
    bool enabledcurvecrosshatching;
    bool enabledcurveopacity;
    bool enabledcurveseparation;
    bool enabledcurvesize;
    bool enabledcurvethickness;

    //Hatching Options
    double angle;
    double separation;
    double thickness;
    double origin_x;
    double origin_y;
    bool nocrosshatching;
    bool perpendicular;
    bool minusthenplus;
    bool plusthenminus;
    bool moirepattern;
    int crosshatchingstyle;
    int separationintervals;

    //Hatching Preferences
    //bool trigonometryalgebra;
    //bool scratchoff;
    bool antialias;
    bool subpixelprecision;
    bool opaquebackground;

    //Angle, Crosshatching, Separation and Thickness curves
    double anglesensorvalue;
    double crosshatchingsensorvalue;
    double separationsensorvalue;
    double thicknesssensorvalue;

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
