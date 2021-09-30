/*
 *  SPDX-FileCopyrightText: 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "kis_particle_paintop_settings.h"

#include "kis_particle_paintop_settings_widget.h"
#include "kis_particleop_option.h"

#include <kis_paint_action_type_option.h>
#include <kis_airbrush_option_widget.h>

struct KisParticlePaintOpSettings::Private
{
    QList<KisUniformPaintOpPropertyWSP> uniformProperties;
};

KisParticlePaintOpSettings::KisParticlePaintOpSettings(KisResourcesInterfaceSP resourcesInterface)
    : KisNoSizePaintOpSettings(resourcesInterface),
      m_d(new Private)
{
}

KisParticlePaintOpSettings::~KisParticlePaintOpSettings()
{
}

bool KisParticlePaintOpSettings::lodSizeThresholdSupported() const
{
    return false;
}

bool KisParticlePaintOpSettings::paintIncremental()
{
    return (enumPaintActionType)getInt("PaintOpAction", WASH) == BUILDUP;
}


#include <brushengine/kis_slider_based_paintop_property.h>
#include "kis_paintop_preset.h"
#include "KisPaintOpPresetUpdateProxy.h"
#include "kis_standard_uniform_properties_factory.h"


QList<KisUniformPaintOpPropertySP> KisParticlePaintOpSettings::uniformProperties(KisPaintOpSettingsSP settings, QPointer<KisPaintOpPresetUpdateProxy> updateProxy)
{
    QList<KisUniformPaintOpPropertySP> props =
        listWeakToStrong(m_d->uniformProperties);

    if (props.isEmpty()) {
        {
            KisIntSliderBasedPaintOpPropertyCallback *prop =
                new KisIntSliderBasedPaintOpPropertyCallback(
                    KisIntSliderBasedPaintOpPropertyCallback::Int,
                    "particle_particles",
                    i18n("Particles"),
                    settings, 0);

            prop->setRange(1, 500);
            prop->setSingleStep(1);

            prop->setReadCallback(
                [](KisUniformPaintOpProperty *prop) {
                    ParticleOption option;
                    option.readOptionSetting(prop->settings().data());

                    prop->setValue(int(option.particle_count));
                });
            prop->setWriteCallback(
                [](KisUniformPaintOpProperty *prop) {
                    ParticleOption option;
                    option.readOptionSetting(prop->settings().data());
                    option.particle_count = prop->value().toInt();
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
                    "particle_opecityweight",
                    i18n("Opacity Weight"),
                    settings, 0);

            prop->setRange(0.01, 1.0);
            prop->setSingleStep(0.01);
            prop->setDecimals(2);

            prop->setReadCallback(
                [](KisUniformPaintOpProperty *prop) {
                    ParticleOption option;
                    option.readOptionSetting(prop->settings().data());
                    prop->setValue(option.particle_weight);
                });
            prop->setWriteCallback(
                [](KisUniformPaintOpProperty *prop) {
                    ParticleOption option;
                    option.readOptionSetting(prop->settings().data());
                    option.particle_weight = prop->value().toReal();
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
                    "particle_dx_scale",
                    i18n("dx scale"),
                    settings, 0);

            prop->setRange(-2, 2);
            prop->setSingleStep(0.01);
            prop->setDecimals(2);

            prop->setReadCallback(
                [](KisUniformPaintOpProperty *prop) {
                    ParticleOption option;
                    option.readOptionSetting(prop->settings().data());
                    prop->setValue(option.particle_scale_x);
                });
            prop->setWriteCallback(
                [](KisUniformPaintOpProperty *prop) {
                    ParticleOption option;
                    option.readOptionSetting(prop->settings().data());
                    option.particle_scale_x = prop->value().toReal();
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
                    "particle_dy_scale",
                    i18n("dy scale"),
                    settings, 0);

            prop->setRange(-2, 2);
            prop->setSingleStep(0.01);
            prop->setDecimals(2);

            prop->setReadCallback(
                [](KisUniformPaintOpProperty *prop) {
                    ParticleOption option;
                    option.readOptionSetting(prop->settings().data());
                    prop->setValue(option.particle_scale_y);
                });
            prop->setWriteCallback(
                [](KisUniformPaintOpProperty *prop) {
                    ParticleOption option;
                    option.readOptionSetting(prop->settings().data());
                    option.particle_scale_y = prop->value().toReal();
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
                    "particle_gravity",
                    i18n("Gravity"),
                    settings, 0);

            prop->setRange(0.01, 1.0);
            prop->setSingleStep(0.01);
            prop->setDecimals(2);

            prop->setReadCallback(
                [](KisUniformPaintOpProperty *prop) {
                    ParticleOption option;
                    option.readOptionSetting(prop->settings().data());
                    prop->setValue(option.particle_gravity);
                });
            prop->setWriteCallback(
                [](KisUniformPaintOpProperty *prop) {
                    ParticleOption option;
                    option.readOptionSetting(prop->settings().data());
                    option.particle_gravity = prop->value().toReal();
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
                    "particle_iterations",
                    i18n("Iterations"),
                    settings, 0);

            prop->setRange(1, 300);
            prop->setSingleStep(1);

            prop->setReadCallback(
                [](KisUniformPaintOpProperty *prop) {
                    ParticleOption option;
                    option.readOptionSetting(prop->settings().data());

                    prop->setValue(int(option.particle_iterations));
                });
            prop->setWriteCallback(
                [](KisUniformPaintOpProperty *prop) {
                    ParticleOption option;
                    option.readOptionSetting(prop->settings().data());
                    option.particle_iterations = prop->value().toInt();
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
