/*
 *  SPDX-FileCopyrightText: 2008, 2009 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include <kis_curve_paintop_settings.h>
#include <kis_paint_action_type_option.h>
#include "kis_curve_line_option.h"

struct KisCurvePaintOpSettings::Private
{
    QList<KisUniformPaintOpPropertyWSP> uniformProperties;
};

KisCurvePaintOpSettings::KisCurvePaintOpSettings(KisResourcesInterfaceSP resourcesInterface)
    : KisPaintOpSettings(resourcesInterface),
      m_d(new Private)
{
}

KisCurvePaintOpSettings::~KisCurvePaintOpSettings()
{
}

void KisCurvePaintOpSettings::setPaintOpSize(qreal value)
{
    KisCurveOptionProperties option;
    option.readOptionSetting(this);
    option.curve_line_width = value;
    option.writeOptionSetting(this);
}

qreal KisCurvePaintOpSettings::paintOpSize() const
{
    KisCurveOptionProperties option;
    option.readOptionSetting(this);
    return option.curve_line_width;
}

bool KisCurvePaintOpSettings::paintIncremental()
{
    return (enumPaintActionType)getInt("PaintOpAction", WASH) == BUILDUP;
}


#include <brushengine/kis_slider_based_paintop_property.h>
#include "kis_paintop_preset.h"
#include "KisPaintOpPresetUpdateProxy.h"
#include "kis_standard_uniform_properties_factory.h"


QList<KisUniformPaintOpPropertySP> KisCurvePaintOpSettings::uniformProperties(KisPaintOpSettingsSP settings, QPointer<KisPaintOpPresetUpdateProxy> updateProxy)
{
    QList<KisUniformPaintOpPropertySP> props =
        listWeakToStrong(m_d->uniformProperties);

    if (props.isEmpty()) {
        {
            KisIntSliderBasedPaintOpPropertyCallback *prop =
                new KisIntSliderBasedPaintOpPropertyCallback(
                    KisIntSliderBasedPaintOpPropertyCallback::Int,
                    "curve_linewidth",
                    i18n("Line Width"),
                    settings, 0);

            prop->setRange(1, 100);
            prop->setSingleStep(1);
            prop->setSuffix(i18n(" px"));

            prop->setReadCallback(
                [](KisUniformPaintOpProperty *prop) {
                    KisCurveOptionProperties option;
                    option.readOptionSetting(prop->settings().data());

                    prop->setValue(option.curve_line_width);
                });
            prop->setWriteCallback(
                [](KisUniformPaintOpProperty *prop) {
                    KisCurveOptionProperties option;
                    option.readOptionSetting(prop->settings().data());
                    option.curve_line_width = prop->value().toInt();
                    option.writeOptionSetting(prop->settings().data());
                });

            QObject::connect(updateProxy, SIGNAL(sigSettingsChanged()), prop, SLOT(requestReadValue()));
            prop->requestReadValue();
            props << toQShared(prop);
        }
        {
            KisIntSliderBasedPaintOpPropertyCallback *prop =
                new KisIntSliderBasedPaintOpPropertyCallback(
                    KisIntSliderBasedPaintOpPropertyCallback::Int,
                    "curve_historysize",
                    i18n("History Size"),
                    settings, 0);

            prop->setRange(2, 300);
            prop->setSingleStep(1);

            prop->setReadCallback(
                [](KisUniformPaintOpProperty *prop) {
                    KisCurveOptionProperties option;
                    option.readOptionSetting(prop->settings().data());

                    prop->setValue(option.curve_stroke_history_size);
                });
            prop->setWriteCallback(
                [](KisUniformPaintOpProperty *prop) {
                    KisCurveOptionProperties option;
                    option.readOptionSetting(prop->settings().data());
                    option.curve_stroke_history_size = prop->value().toInt();
                    option.writeOptionSetting(prop->settings().data());
                });
            QObject::connect(updateProxy, SIGNAL(sigSettingsChanged()), prop, SLOT(requestReadValue()));
            prop->requestReadValue();
            props << toQShared(prop);
        }

        {
            KisDoubleSliderBasedPaintOpPropertyCallback *prop =
                new KisDoubleSliderBasedPaintOpPropertyCallback(
                    KisDoubleSliderBasedPaintOpPropertyCallback::Double,
                    "curve_lineopacity",
                    i18n("Line Opacity"),
                    settings, 0);

            prop->setRange(0, 100.0);
            prop->setSingleStep(0.01);
            prop->setDecimals(2);
            prop->setSuffix(i18n("%"));

            prop->setReadCallback(
                [](KisUniformPaintOpProperty *prop) {
                    KisCurveOptionProperties option;
                    option.readOptionSetting(prop->settings().data());
                    prop->setValue(option.curve_curves_opacity * 100.0);
                });
            prop->setWriteCallback(
                [](KisUniformPaintOpProperty *prop) {
                    KisCurveOptionProperties option;
                    option.readOptionSetting(prop->settings().data());
                    option.curve_curves_opacity = prop->value().toReal() / 100.0;
                    option.writeOptionSetting(prop->settings().data());
                });

            QObject::connect(updateProxy, SIGNAL(sigSettingsChanged()), prop, SLOT(requestReadValue()));
            prop->requestReadValue();
            props << toQShared(prop);
        }

        {
            KisUniformPaintOpPropertyCallback *prop =
                new KisUniformPaintOpPropertyCallback(
                    KisUniformPaintOpPropertyCallback::Bool,
                    "curve_connectionline",
                    i18n("Connection Line"),
                    settings, 0);

            prop->setReadCallback(
                [](KisUniformPaintOpProperty *prop) {
                    KisCurveOptionProperties option;
                    option.readOptionSetting(prop->settings().data());

                    prop->setValue(option.curve_paint_connection_line);
                });
            prop->setWriteCallback(
                [](KisUniformPaintOpProperty *prop) {
                    KisCurveOptionProperties option;
                    option.readOptionSetting(prop->settings().data());
                    option.curve_paint_connection_line = prop->value().toBool();
                    option.writeOptionSetting(prop->settings().data());
                });

            QObject::connect(updateProxy, SIGNAL(sigSettingsChanged()), prop, SLOT(requestReadValue()));
            prop->requestReadValue();
            props << toQShared(prop);
        }
    }

    {
        using namespace KisStandardUniformPropertiesFactory;

        Q_FOREACH (KisUniformPaintOpPropertySP prop, KisPaintOpSettings::uniformProperties(settings, updateProxy)) {
            if (prop->id() == opacity.id()) {
                props.prepend(prop);
            }
        }
    }

    return props;
}
