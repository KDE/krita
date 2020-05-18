/*
 *  Copyright (c) 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *  Copyright (c) 2010 José Luis Vergara <pentalis@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "kis_hatching_paintop_settings_widget.h"

#include "kis_hatching_options.h"
#include "kis_hatching_preferences.h"
#include "kis_hatching_paintop_settings.h"

#include "kis_hatching_pressure_angle_option.h"
#include "kis_hatching_pressure_crosshatching_option.h"
#include "kis_hatching_pressure_separation_option.h"
#include "kis_hatching_pressure_thickness_option.h"

#include <kis_brush_option_widget.h>
#include <kis_curve_option_widget.h>
#include <kis_pressure_opacity_option.h>
#include <kis_pressure_size_option.h>
#include <kis_paintop_settings_widget.h>
#include <kis_paint_action_type_option.h>
#include <kis_compositeop_option.h>
#include "kis_texture_option.h"
#include <kis_pressure_mirror_option_widget.h>
#include "kis_pressure_texture_strength_option.h"

#include <QDomDocument>
#include <QDomElement>

KisHatchingPaintOpSettingsWidget:: KisHatchingPaintOpSettingsWidget(QWidget* parent)
    : KisBrushBasedPaintopOptionWidget(parent)
{
    setPrecisionEnabled(true);

    //-------Adding widgets to the screen------------

    addPaintOpOption(new KisHatchingOptions(), i18n("Hatching options"));
    addPaintOpOption(new KisHatchingPreferences(), i18n("Hatching preferences"));
    addPaintOpOption(new KisCompositeOpOption(true), i18n("Blending Mode"));
    addPaintOpOption(new KisCurveOptionWidget(new KisHatchingPressureSeparationOption(), i18n("0.0"), i18n("1.0")), i18n("Separation"));
    addPaintOpOption(new KisCurveOptionWidget(new KisHatchingPressureThicknessOption(), i18n("0.0"), i18n("1.0")), i18n("Thickness"));
    addPaintOpOption(new KisCurveOptionWidget(new KisHatchingPressureAngleOption(), i18n("0.0"), i18n("1.0")), i18n("Angle"));
    addPaintOpOption(new KisCurveOptionWidget(new KisHatchingPressureCrosshatchingOption(), i18n("0.0"), i18n("1.0")), i18n("Crosshatching"));
    addPaintOpOption(new KisCurveOptionWidget(new KisPressureOpacityOption(), i18n("Transparent"), i18n("Opaque")), i18n("Opacity"));
    addPaintOpOption(new KisCurveOptionWidget(new KisPressureSizeOption(), i18n("0%"), i18n("100%")), i18n("Size"));
    addPaintOpOption(new KisPressureMirrorOptionWidget(), i18n("Mirror"));

    addPaintOpOption(new KisPaintActionTypeOption(), i18n("Painting Mode"));

    addPaintOpOption(new KisTextureOption(), i18n("Pattern"));
        addPaintOpOption(new KisCurveOptionWidget(new KisPressureTextureStrengthOption(), i18n("Weak"), i18n("Strong")), i18n("Strength"));

    //-----Useful to read first:------
    /*
    Below you will encounter a reasonably correct solution to the problem of changing
    the default presets of the "BrushTip" popup configuration dialogue.
    In my (Pentalis) opinion, the best solution is code refactoring (simpler ways
    to change the defaults). On the meanwhile, copypasting this code
    won't give your class a charisma penalty.
    In kis_hatching_paintop_settings.cpp you will find a snippet of code to
    discover the structure of your XML config tree if you need to edit it at build
    time like here.
    */

    //---------START ALTERING DEFAULT VALUES-----------

    //As the name implies, reconfigurationCourier is the KisPropertiesConfigurationSP
    //we'll use as an intermediary to edit the default settings
    KisPropertiesConfigurationSP reconfigurationCourier = configuration();

    /*xMLAnalyzer is an empty document we'll use to analyze and edit the config string part by part
    I know the important string is "brush_definition" because I read the tree with the snippet
    in kis_hatching_paintop_settings.cpp */
    QDomDocument xMLAnalyzer;
    xMLAnalyzer.setContent(reconfigurationCourier->getString("brush_definition"));

    /*More things I know by reading the XML tree. At this point you can just read it with:
    dbgKrita << xMLAnalyzer.toString() ;
    those QDomElements are the way to navigate the XML tree, read
    https://doc.qt.io/qt-5/qdomdocument.html for more information */
    QDomElement firstTag = xMLAnalyzer.documentElement();
    QDomElement firstTagsChild = firstTag.elementsByTagName("MaskGenerator").item(0).toElement();

    // SET THE DEFAULT VALUES
    firstTag.attributeNode("spacing").setValue("0.4");
    firstTagsChild.attributeNode("diameter").setValue("30");

    //Write them into the intermediary config file
    reconfigurationCourier->setProperty("brush_definition", xMLAnalyzer.toString());

    KisCubicCurve CurveSize;

    CurveSize.fromString("0,1;1,0.1;");
    //dbgKrita << "\n\n\n" << CurveSize.toString() << "\n\n\n";

    QVariant QVCurveSize = QVariant::fromValue(CurveSize);

    reconfigurationCourier->setProperty("CurveSize", QVCurveSize);

    setConfiguration(reconfigurationCourier);  // Finished.

    /* Debugging block
    QMap<QString, QVariant> rofl = QMap<QString, QVariant>(reconfigurationCourier->getProperties());

    QMap<QString, QVariant>::const_iterator i;
    for (i = rofl.constBegin(); i != rofl.constEnd(); ++i)
        dbgKrita << i.key() << ":" << i.value();
    */

}

KisHatchingPaintOpSettingsWidget::~ KisHatchingPaintOpSettingsWidget()
{
}

KisPropertiesConfigurationSP  KisHatchingPaintOpSettingsWidget::configuration() const
{
    KisHatchingPaintOpSettingsSP config = new KisHatchingPaintOpSettings();
    config->setOptionsWidget(const_cast<KisHatchingPaintOpSettingsWidget*>(this));
    config->setProperty("paintop", "hatchingbrush"); // XXX: make this a const id string
    writeConfiguration(config);
    return config;
}
