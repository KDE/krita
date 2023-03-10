/*
 *  SPDX-FileCopyrightText: 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *  SPDX-FileCopyrightText: 2010 José Luis Vergara <pentalis@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_hatching_paintop_settings.h"

struct KisHatchingPaintOpSettings::Private
{
    QList<KisUniformPaintOpPropertyWSP> uniformProperties;
};


KisHatchingPaintOpSettings::KisHatchingPaintOpSettings(KisResourcesInterfaceSP resourcesInterface)
    : KisBrushBasedPaintOpSettings(resourcesInterface),
      m_d(new Private)
{
}

KisHatchingPaintOpSettings::~KisHatchingPaintOpSettings()
{
}

void KisHatchingPaintOpSettings::initializeTwin(KisPaintOpSettingsSP settings) const
{
    // XXX: this is a nice way to reinvent the copy constructor?

    /*--------DO NOT REMOVE please, use this to review the XML config tree
    QMap<QString, QVariant> rofl = QMap<QString, QVariant>(getProperties());

    QMap<QString, QVariant>::const_iterator i;
    for (i = rofl.constBegin(); i != rofl.constEnd(); ++i)
        dbgKrita << i.key() << ":" << i.value();
    /----------DO NOT REMOVE----------------*/

    KisHatchingPaintOpSettings *convenienttwin = static_cast<KisHatchingPaintOpSettings*>(settings.data());

    convenienttwin->enabledcurveangle = getBool("PressureAngle");
    convenienttwin->enabledcurvecrosshatching = getBool("PressureCrosshatching");
    convenienttwin->enabledcurveopacity = getBool("PressureOpacity");
    convenienttwin->enabledcurveseparation = getBool("PressureSeparation");
    convenienttwin->enabledcurvesize = getBool("PressureSize");
    convenienttwin->enabledcurvethickness = getBool("PressureThickness");

    convenienttwin->angle = getDouble("Hatching/angle");
    convenienttwin->separation = getDouble("Hatching/separation");
    convenienttwin->thickness = getDouble("Hatching/thickness");
    convenienttwin->origin_x = getDouble("Hatching/origin_x");
    convenienttwin->origin_y = getDouble("Hatching/origin_y");

    convenienttwin->nocrosshatching = getBool("Hatching/bool_nocrosshatching");
    convenienttwin->perpendicular = getBool("Hatching/bool_perpendicular");
    convenienttwin->minusthenplus = getBool("Hatching/bool_minusthenplus");
    convenienttwin->plusthenminus = getBool("Hatching/bool_plusthenminus");
    convenienttwin->moirepattern = getBool("Hatching/bool_moirepattern");

    convenienttwin->separationintervals = getInt("Hatching/separationintervals");

    //convenienttwin->trigonometryalgebra = getBool("Hatching/bool_trigonometryalgebra");
    //convenienttwin->scratchoff = getBool("Hatching/bool_scratchoff");
    convenienttwin->antialias = getBool("Hatching/bool_antialias");
    convenienttwin->opaquebackground = getBool("Hatching/bool_opaquebackground");
    convenienttwin->subpixelprecision = getBool("Hatching/bool_subpixelprecision");

    if (getBool("Hatching/bool_nocrosshatching"))
        convenienttwin->crosshatchingstyle = 0;
    else if (getBool("Hatching/bool_perpendicular"))
        convenienttwin->crosshatchingstyle = 1;
    else if (getBool("Hatching/bool_minusthenplus"))
        convenienttwin->crosshatchingstyle = 2;
    else if (getBool("Hatching/bool_plusthenminus"))
        convenienttwin->crosshatchingstyle = 3;
    if (getBool("Hatching/bool_moirepattern"))
        convenienttwin->crosshatchingstyle = 4;

}


#include <brushengine/kis_slider_based_paintop_property.h>
#include "kis_paintop_preset.h"
#include "KisPaintOpPresetUpdateProxy.h"
#include "KisHatchingOptionsData.h"


QList<KisUniformPaintOpPropertySP> KisHatchingPaintOpSettings::uniformProperties(KisPaintOpSettingsSP settings, QPointer<KisPaintOpPresetUpdateProxy> updateProxy)
{
    QList<KisUniformPaintOpPropertySP> props =
        listWeakToStrong(m_d->uniformProperties);

    if (props.isEmpty()) {
        {
            KisDoubleSliderBasedPaintOpPropertyCallback *prop =
                new KisDoubleSliderBasedPaintOpPropertyCallback(KisDoubleSliderBasedPaintOpPropertyCallback::Double,
                                                                KisDoubleSliderBasedPaintOpPropertyCallback::SubType_Angle,
                                                                KoID("hatching_angle", i18n("Hatching Angle")),
                                                                settings,
                                                                0);

            const QString degree = QChar(Qt::Key_degree);
            prop->setRange(-90, 90);
            prop->setSingleStep(0.01);
            prop->setDecimals(2);
            prop->setSuffix(degree);

            prop->setReadCallback(
                [](KisUniformPaintOpProperty *prop) {
                    KisHatchingOptionsData option;
                    option.read(prop->settings().data());
                    prop->setValue(option.angle);
                });
            prop->setWriteCallback(
                [](KisUniformPaintOpProperty *prop) {
                    KisHatchingOptionsData option;
                    option.read(prop->settings().data());
                    option.angle = prop->value().toReal();
                    option.write(prop->settings().data());
                });

            QObject::connect(updateProxy, SIGNAL(sigSettingsChanged()), prop, SLOT(requestReadValue()));
            prop->requestReadValue();
            props << toQShared(prop);
        }
        {
            KisDoubleSliderBasedPaintOpPropertyCallback *prop =
                new KisDoubleSliderBasedPaintOpPropertyCallback(KisDoubleSliderBasedPaintOpPropertyCallback::Double,
                                                                KoID("hatching_separation", i18n("Separation")),
                                                                settings,
                                                                0);

            prop->setRange(1.0, 30);
            prop->setSingleStep(0.01);
            prop->setDecimals(2);
            prop->setSuffix(i18n(" px"));

            prop->setReadCallback(
                [](KisUniformPaintOpProperty *prop) {
                    KisHatchingOptionsData option;
                    option.read(prop->settings().data());
                    prop->setValue(option.separation);
                });
            prop->setWriteCallback(
                [](KisUniformPaintOpProperty *prop) {
                    KisHatchingOptionsData option;
                    option.read(prop->settings().data());
                    option.separation = prop->value().toReal();
                    option.write(prop->settings().data());
                });

            QObject::connect(updateProxy, SIGNAL(sigSettingsChanged()), prop, SLOT(requestReadValue()));
            prop->requestReadValue();
            props << toQShared(prop);
        }
        {
            KisDoubleSliderBasedPaintOpPropertyCallback *prop =
                new KisDoubleSliderBasedPaintOpPropertyCallback(KisDoubleSliderBasedPaintOpPropertyCallback::Double,
                                                                KoID("hatching_thickness", i18n("Thickness")),
                                                                settings,
                                                                0);

            prop->setRange(1.0, 30);
            prop->setSingleStep(0.01);
            prop->setDecimals(2);
            prop->setSuffix(i18n(" px"));

            prop->setReadCallback(
                [](KisUniformPaintOpProperty *prop) {
                    KisHatchingOptionsData option;
                    option.read(prop->settings().data());
                    prop->setValue(option.thickness);
                });
            prop->setWriteCallback(
                [](KisUniformPaintOpProperty *prop) {
                    KisHatchingOptionsData option;
                    option.read(prop->settings().data());
                    option.thickness = prop->value().toReal();
                    option.write(prop->settings().data());
                });

            QObject::connect(updateProxy, SIGNAL(sigSettingsChanged()), prop, SLOT(requestReadValue()));
            prop->requestReadValue();
            props << toQShared(prop);
        }
    }

    return KisPaintOpSettings::uniformProperties(settings, updateProxy) + props;
}

