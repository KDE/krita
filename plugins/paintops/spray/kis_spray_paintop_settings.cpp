/*
 *  SPDX-FileCopyrightText: 2008, 2009, 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <cmath>

#include <kis_paint_action_type_option.h>
#include <kis_color_option.h>

#include "kis_spray_paintop_settings.h"
#include "kis_sprayop_option.h"
#include "kis_spray_shape_option.h"
#include <kis_airbrush_option_widget.h>

struct KisSprayPaintOpSettings::Private
{
    QList<KisUniformPaintOpPropertyWSP> uniformProperties;
};


KisSprayPaintOpSettings::KisSprayPaintOpSettings(KisResourcesInterfaceSP resourcesInterface)
    : KisOutlineGenerationPolicy<KisPaintOpSettings>(KisCurrentOutlineFetcher::SIZE_OPTION |
                                                     KisCurrentOutlineFetcher::ROTATION_OPTION,
                                                     resourcesInterface),
    m_d(new Private)
{
}

KisSprayPaintOpSettings::~KisSprayPaintOpSettings()
{
}

void KisSprayPaintOpSettings::setPaintOpSize(qreal value)
{
    KisSprayOptionProperties option;
    option.readOptionSetting(this);
    option.diameter = value;
    option.writeOptionSetting(this);
}

qreal KisSprayPaintOpSettings::paintOpSize() const
{

    KisSprayOptionProperties option;
    option.readOptionSetting(this);

    return option.diameter;
}

bool KisSprayPaintOpSettings::paintIncremental()
{
    return (enumPaintActionType)getInt("PaintOpAction", WASH) == BUILDUP;
}


QPainterPath KisSprayPaintOpSettings::brushOutline(const KisPaintInformation &info, const OutlineMode &mode, qreal alignForZoom)
{
    QPainterPath path;
    if (mode.isVisible) {
        qreal width = getInt(SPRAY_DIAMETER);
        qreal height = getInt(SPRAY_DIAMETER) * getDouble(SPRAY_ASPECT);
        path = ellipseOutline(width, height, getDouble(SPRAY_SCALE), getDouble(SPRAY_ROTATION));

        path = outlineFetcher()->fetchOutline(info, this, path, mode, alignForZoom);

        if (mode.forceFullSize) {
            QPainterPath tiltLine =
                makeTiltIndicator(info, QPointF(0.0, 0.0), width * 0.5, 3.0);
            path.addPath(outlineFetcher()->fetchOutline(info, this, tiltLine, mode, alignForZoom, 1.0, 0.0, true, 0, 0));
        }
    }
    return path;
}

#include <brushengine/kis_slider_based_paintop_property.h>
#include "kis_paintop_preset.h"
#include "KisPaintOpPresetUpdateProxy.h"
#include "kis_standard_uniform_properties_factory.h"


QList<KisUniformPaintOpPropertySP> KisSprayPaintOpSettings::uniformProperties(KisPaintOpSettingsSP settings, QPointer<KisPaintOpPresetUpdateProxy> updateProxy)
{
    QList<KisUniformPaintOpPropertySP> props =
        listWeakToStrong(m_d->uniformProperties);

    if (props.isEmpty()) {
        {
            KisDoubleSliderBasedPaintOpPropertyCallback *prop =
                new KisDoubleSliderBasedPaintOpPropertyCallback(
                    KisDoubleSliderBasedPaintOpPropertyCallback::Double,
                    "spacing",
                    i18n("Spacing"),
                    settings, 0);

            prop->setRange(0.01, 10);
            prop->setSingleStep(0.01);
            prop->setExponentRatio(3.0);

            prop->setReadCallback(
                [](KisUniformPaintOpProperty *prop) {
                    KisSprayOptionProperties option;
                    option.readOptionSetting(prop->settings().data());

                    prop->setValue(option.spacing);
                });
            prop->setWriteCallback(
                [](KisUniformPaintOpProperty *prop) {
                    KisSprayOptionProperties option;
                    option.readOptionSetting(prop->settings().data());
                    option.spacing = prop->value().toReal();
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
                    "spray_particlecount",
                    i18n("Particle Count"),
                    settings, 0);

            prop->setRange(0, 1000);
            prop->setExponentRatio(3);

            prop->setReadCallback(
                [](KisUniformPaintOpProperty *prop) {
                    KisSprayOptionProperties option;
                    option.readOptionSetting(prop->settings().data());

                    prop->setValue(int(option.particleCount));
                });
            prop->setWriteCallback(
                [](KisUniformPaintOpProperty *prop) {
                    KisSprayOptionProperties option;
                    option.readOptionSetting(prop->settings().data());
                    option.particleCount = prop->value().toInt();
                    option.writeOptionSetting(prop->settings().data());
                });
            prop->setIsVisibleCallback(
                [](const KisUniformPaintOpProperty *prop) {
                    KisSprayOptionProperties option;
                    option.readOptionSetting(prop->settings().data());
                    return !option.useDensity;
                });

            QObject::connect(updateProxy, SIGNAL(sigSettingsChanged()), prop, SLOT(requestReadValue()));
            prop->requestReadValue();
            props << toQShared(prop);
        }
        {
            KisDoubleSliderBasedPaintOpPropertyCallback *prop =
                new KisDoubleSliderBasedPaintOpPropertyCallback(
                    KisDoubleSliderBasedPaintOpPropertyCallback::Double,
                    "spray_density",
                    i18n("Density"),
                    settings, 0);

            prop->setRange(0.1, 100);
            prop->setSingleStep(0.01);
            prop->setDecimals(2);
            prop->setExponentRatio(3);
            prop->setSuffix(i18n("%"));

            prop->setReadCallback(
                [](KisUniformPaintOpProperty *prop) {
                    KisSprayOptionProperties option;
                    option.readOptionSetting(prop->settings().data());
                    prop->setValue(option.coverage);
                });
            prop->setWriteCallback(
                [](KisUniformPaintOpProperty *prop) {
                    KisSprayOptionProperties option;
                    option.readOptionSetting(prop->settings().data());
                    option.coverage = prop->value().toReal();
                    option.writeOptionSetting(prop->settings().data());
                });
            prop->setIsVisibleCallback(
                [](const KisUniformPaintOpProperty *prop) {
                    KisSprayOptionProperties option;
                    option.readOptionSetting(prop->settings().data());
                    return option.useDensity;
                });
            QObject::connect(updateProxy, SIGNAL(sigSettingsChanged()), prop, SLOT(requestReadValue()));
            prop->requestReadValue();
            props << toQShared(prop);
        }
    }
    {
        using namespace KisStandardUniformPropertiesFactory;

        Q_FOREACH (KisUniformPaintOpPropertySP prop, KisPaintOpSettings::uniformProperties(settings, updateProxy)) {
            if (prop->id() == opacity.id() ||
                prop->id() == size.id()) {

                props.prepend(prop);
            }
        }
    }

    return props;
}
