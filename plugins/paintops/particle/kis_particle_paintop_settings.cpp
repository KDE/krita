/*
 *  SPDX-FileCopyrightText: 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "kis_particle_paintop_settings.h"

#include "kis_particle_paintop_settings_widget.h"
#include "KisParticleOpOptionData.h"

#include <KisPaintingModeOptionData.h>

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

bool KisParticlePaintOpSettings::paintIncremental()
{
    KisPaintingModeOptionData data;
    data.read(this);
    return data.paintingMode == enumPaintingMode::BUILDUP;
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
            KisIntSliderBasedPaintOpPropertyCallback *prop = new KisIntSliderBasedPaintOpPropertyCallback(KisIntSliderBasedPaintOpPropertyCallback::Int,
                                                                                                          KoID("particle_particles", i18n("Particles")),
                                                                                                          settings,
                                                                                                          0);

            prop->setRange(1, 500);
            prop->setSingleStep(1);

            prop->setReadCallback(
                [](KisUniformPaintOpProperty *prop) {
                    KisParticleOpOptionData option;
                    option.read(prop->settings().data());

                    prop->setValue(int(option.particleCount));
                });
            prop->setWriteCallback(
                [](KisUniformPaintOpProperty *prop) {
                    KisParticleOpOptionData option;
                    option.read(prop->settings().data());
                    option.particleCount = prop->value().toInt();
                    option.write(prop->settings().data());
                });

            QObject::connect(updateProxy, SIGNAL(sigSettingsChanged()), prop, SLOT(requestReadValue()));
            prop->requestReadValue();
            props << toQShared(prop);
        }
        {
            KisDoubleSliderBasedPaintOpPropertyCallback *prop =
                new KisDoubleSliderBasedPaintOpPropertyCallback(KisDoubleSliderBasedPaintOpPropertyCallback::Double,
                                                                KoID("particle_opecityweight", i18n("Opacity Weight")),
                                                                settings,
                                                                0);

            prop->setRange(0.01, 1.0);
            prop->setSingleStep(0.01);
            prop->setDecimals(2);

            prop->setReadCallback(
                [](KisUniformPaintOpProperty *prop) {
                    KisParticleOpOptionData option;
                    option.read(prop->settings().data());
                    prop->setValue(option.particleWeight);
                });
            prop->setWriteCallback(
                [](KisUniformPaintOpProperty *prop) {
                    KisParticleOpOptionData option;
                    option.read(prop->settings().data());
                    option.particleWeight = prop->value().toReal();
                    option.write(prop->settings().data());
                });

            QObject::connect(updateProxy, SIGNAL(sigSettingsChanged()), prop, SLOT(requestReadValue()));
            prop->requestReadValue();
            props << toQShared(prop);
        }
        {
            KisDoubleSliderBasedPaintOpPropertyCallback *prop =
                new KisDoubleSliderBasedPaintOpPropertyCallback(KisDoubleSliderBasedPaintOpPropertyCallback::Double,
                                                                KoID("particle_dx_scale", i18n("dx scale")),
                                                                settings,
                                                                0);

            prop->setRange(-2, 2);
            prop->setSingleStep(0.01);
            prop->setDecimals(2);

            prop->setReadCallback(
                [](KisUniformPaintOpProperty *prop) {
                    KisParticleOpOptionData option;
                    option.read(prop->settings().data());
                    prop->setValue(option.particleScaleX);
                });
            prop->setWriteCallback(
                [](KisUniformPaintOpProperty *prop) {
                    KisParticleOpOptionData option;
                    option.read(prop->settings().data());
                    option.particleScaleX = prop->value().toReal();
                    option.write(prop->settings().data());
                });

            QObject::connect(updateProxy, SIGNAL(sigSettingsChanged()), prop, SLOT(requestReadValue()));
            prop->requestReadValue();
            props << toQShared(prop);
        }
        {
            KisDoubleSliderBasedPaintOpPropertyCallback *prop =
                new KisDoubleSliderBasedPaintOpPropertyCallback(KisDoubleSliderBasedPaintOpPropertyCallback::Double,
                                                                KoID("particle_dy_scale", i18n("dy scale")),
                                                                settings,
                                                                0);

            prop->setRange(-2, 2);
            prop->setSingleStep(0.01);
            prop->setDecimals(2);

            prop->setReadCallback(
                [](KisUniformPaintOpProperty *prop) {
                    KisParticleOpOptionData option;
                    option.read(prop->settings().data());
                    prop->setValue(option.particleScaleY);
                });
            prop->setWriteCallback(
                [](KisUniformPaintOpProperty *prop) {
                    KisParticleOpOptionData option;
                    option.read(prop->settings().data());
                    option.particleScaleY = prop->value().toReal();
                    option.write(prop->settings().data());
                });

            QObject::connect(updateProxy, SIGNAL(sigSettingsChanged()), prop, SLOT(requestReadValue()));
            prop->requestReadValue();
            props << toQShared(prop);
        }
        {
            KisDoubleSliderBasedPaintOpPropertyCallback *prop =
                new KisDoubleSliderBasedPaintOpPropertyCallback(KisDoubleSliderBasedPaintOpPropertyCallback::Double,
                                                                KoID("particle_gravity", i18n("Gravity")),
                                                                settings,
                                                                0);

            prop->setRange(0.01, 1.0);
            prop->setSingleStep(0.01);
            prop->setDecimals(2);

            prop->setReadCallback(
                [](KisUniformPaintOpProperty *prop) {
                    KisParticleOpOptionData option;
                    option.read(prop->settings().data());
                    prop->setValue(option.particleGravity);
                });
            prop->setWriteCallback(
                [](KisUniformPaintOpProperty *prop) {
                    KisParticleOpOptionData option;
                    option.read(prop->settings().data());
                    option.particleGravity = prop->value().toReal();
                    option.write(prop->settings().data());
                });

            QObject::connect(updateProxy, SIGNAL(sigSettingsChanged()), prop, SLOT(requestReadValue()));
            prop->requestReadValue();
            props << toQShared(prop);
        }
        {
            KisIntSliderBasedPaintOpPropertyCallback *prop = new KisIntSliderBasedPaintOpPropertyCallback(KisIntSliderBasedPaintOpPropertyCallback::Int,
                                                                                                          KoID("particle_iterations", i18n("Iterations")),
                                                                                                          settings,
                                                                                                          0);

            prop->setRange(1, 300);
            prop->setSingleStep(1);

            prop->setReadCallback(
                [](KisUniformPaintOpProperty *prop) {
                    KisParticleOpOptionData option;
                    option.read(prop->settings().data());

                    prop->setValue(int(option.particleIterations));
                });
            prop->setWriteCallback(
                [](KisUniformPaintOpProperty *prop) {
                    KisParticleOpOptionData option;
                    option.read(prop->settings().data());
                    option.particleIterations = prop->value().toInt();
                    option.write(prop->settings().data());
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
