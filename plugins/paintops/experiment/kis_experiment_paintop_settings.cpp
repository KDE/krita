/*
 *  Copyright (c) 2009,2010 Lukáš Tvrdý <lukast.dev@gmail.com>
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

#include "kis_experiment_paintop_settings.h"
#include "kis_current_outline_fetcher.h"

struct KisExperimentPaintOpSettings::Private
{
    QList<KisUniformPaintOpPropertyWSP> uniformProperties;
};

KisExperimentPaintOpSettings::KisExperimentPaintOpSettings()
    : m_d(new Private)
{
}

KisExperimentPaintOpSettings::~KisExperimentPaintOpSettings()
{
}

bool KisExperimentPaintOpSettings::lodSizeThresholdSupported() const
{
    return false;
}

bool KisExperimentPaintOpSettings::paintIncremental()
{
    /**
     * The experiment brush supports working in the
     * WASH mode only!
     */
    return false;
}

QPainterPath KisExperimentPaintOpSettings::brushOutline(const KisPaintInformation &info, const OutlineMode &mode)
{
    QPainterPath path;
    if (mode.isVisible) {

        QRectF ellipse(0, 0, 3, 3);
        ellipse.translate(-ellipse.center());
        path.addEllipse(ellipse);
        ellipse.setRect(0,0, 12, 12);
        ellipse.translate(-ellipse.center());
        path.addEllipse(ellipse);

        if (mode.showTiltDecoration) {
            path.addPath(makeTiltIndicator(info, QPointF(0.0, 0.0), 0.0, 3.0));
        }

        path.translate(info.pos());
    }
    return path;
}

#include <brushengine/kis_slider_based_paintop_property.h>
#include "kis_paintop_preset.h"
#include "kis_paintop_settings_update_proxy.h"
#include "kis_experimentop_option.h"
#include "kis_standard_uniform_properties_factory.h"


QList<KisUniformPaintOpPropertySP> KisExperimentPaintOpSettings::uniformProperties(KisPaintOpSettingsSP settings)
{
    QList<KisUniformPaintOpPropertySP> props =
        listWeakToStrong(m_d->uniformProperties);

    if (props.isEmpty()) {
        {
            KisIntSliderBasedPaintOpPropertyCallback *prop =
                new KisIntSliderBasedPaintOpPropertyCallback(
                    KisIntSliderBasedPaintOpPropertyCallback::Int,
                    "shape_speed",
                    i18n("Speed"),
                    settings, 0);

            prop->setRange(0, 100);
            prop->setSingleStep(1);
            prop->setSuffix(i18n("%"));

            prop->setReadCallback(
                [](KisUniformPaintOpProperty *prop) {
                    ExperimentOption option;
                    option.readOptionSetting(prop->settings().data());

                    prop->setValue(int(option.speed));
                });
            prop->setWriteCallback(
                [](KisUniformPaintOpProperty *prop) {
                    ExperimentOption option;
                    option.readOptionSetting(prop->settings().data());
                    option.speed = prop->value().toInt();
                    option.writeOptionSetting(prop->settings().data());
                });
            prop->setIsVisibleCallback(
                [](const KisUniformPaintOpProperty *prop) -> bool {
                    ExperimentOption option;
                    option.readOptionSetting(prop->settings().data());
                    return option.isSpeedEnabled;
                });

            KisPaintOpPresetSP p = preset().toStrongRef();
            QObject::connect(p->updateProxy(), SIGNAL(sigSettingsChanged()), prop, SLOT(requestReadValue()));
            prop->requestReadValue();
            props << toQShared(prop);
        }
        {
            KisIntSliderBasedPaintOpPropertyCallback *prop =
                new KisIntSliderBasedPaintOpPropertyCallback(
                    KisIntSliderBasedPaintOpPropertyCallback::Int,
                    "shape_smooth",
                    i18n("Smooth"),
                    settings, 0);

            prop->setRange(0, 100);
            prop->setSingleStep(1);
            prop->setSuffix(i18n(" px"));

            prop->setReadCallback(
                [](KisUniformPaintOpProperty *prop) {
                    ExperimentOption option;
                    option.readOptionSetting(prop->settings().data());

                    prop->setValue(int(option.smoothing));
                });
            prop->setWriteCallback(
                [](KisUniformPaintOpProperty *prop) {
                    ExperimentOption option;
                    option.readOptionSetting(prop->settings().data());
                    option.smoothing = prop->value().toInt();
                    option.writeOptionSetting(prop->settings().data());
                });
            prop->setIsVisibleCallback(
                [](const KisUniformPaintOpProperty *prop) {
                    ExperimentOption option;
                    option.readOptionSetting(prop->settings().data());
                    return option.isSmoothingEnabled;
                });

            KisPaintOpPresetSP p = preset().toStrongRef();
            QObject::connect(p->updateProxy(), SIGNAL(sigSettingsChanged()), prop, SLOT(requestReadValue()));
            prop->requestReadValue();
            props << toQShared(prop);
        }

        {
            KisIntSliderBasedPaintOpPropertyCallback *prop =
                new KisIntSliderBasedPaintOpPropertyCallback(
                    KisIntSliderBasedPaintOpPropertyCallback::Int,
                    "shape_displace",
                    i18n("Displace"),
                    settings, 0);

            prop->setRange(0, 100);
            prop->setSingleStep(1);
            prop->setSuffix(i18n("%"));

            prop->setReadCallback(
                [](KisUniformPaintOpProperty *prop) {
                    ExperimentOption option;
                    option.readOptionSetting(prop->settings().data());

                    prop->setValue(int(option.displacement));
                });
            prop->setWriteCallback(
                [](KisUniformPaintOpProperty *prop) {
                    ExperimentOption option;
                    option.readOptionSetting(prop->settings().data());
                    option.displacement = prop->value().toInt();
                    option.writeOptionSetting(prop->settings().data());
                });
            prop->setIsVisibleCallback(
                [](const KisUniformPaintOpProperty *prop) {
                    ExperimentOption option;
                    option.readOptionSetting(prop->settings().data());
                    return option.isDisplacementEnabled;
                });

            KisPaintOpPresetSP p = preset().toStrongRef();
            QObject::connect(p->updateProxy(), SIGNAL(sigSettingsChanged()), prop, SLOT(requestReadValue()));
            prop->requestReadValue();
            props << toQShared(prop);
        }

        {
            KisUniformPaintOpPropertyCallback *prop =
                new KisUniformPaintOpPropertyCallback(
                    KisUniformPaintOpPropertyCallback::Bool,
                    "shape_windingfill",
                    i18n("Winding Fill"),
                    settings, 0);

            prop->setReadCallback(
                [](KisUniformPaintOpProperty *prop) {
                    ExperimentOption option;
                    option.readOptionSetting(prop->settings().data());

                    prop->setValue(option.windingFill);
                });
            prop->setWriteCallback(
                [](KisUniformPaintOpProperty *prop) {
                    ExperimentOption option;
                    option.readOptionSetting(prop->settings().data());
                    option.windingFill = prop->value().toBool();
                    option.writeOptionSetting(prop->settings().data());
                });

            KisPaintOpPresetSP p = preset().toStrongRef();
            QObject::connect(p->updateProxy(), SIGNAL(sigSettingsChanged()), prop, SLOT(requestReadValue()));
            prop->requestReadValue();
            props << toQShared(prop);
        }

        {
            KisUniformPaintOpPropertyCallback *prop =
                new KisUniformPaintOpPropertyCallback(
                    KisUniformPaintOpPropertyCallback::Bool,
                    "shape_hardedge",
                    i18n("Hard Edge"),
                    settings, 0);

            prop->setReadCallback(
                [](KisUniformPaintOpProperty *prop) {
                    ExperimentOption option;
                    option.readOptionSetting(prop->settings().data());

                    prop->setValue(option.hardEdge);
                });
            prop->setWriteCallback(
                [](KisUniformPaintOpProperty *prop) {
                    ExperimentOption option;
                    option.readOptionSetting(prop->settings().data());
                    option.hardEdge = prop->value().toBool();
                    option.writeOptionSetting(prop->settings().data());
                });

            KisPaintOpPresetSP p = preset().toStrongRef();
            QObject::connect(p->updateProxy(), SIGNAL(sigSettingsChanged()), prop, SLOT(requestReadValue()));
            prop->requestReadValue();
            props << toQShared(prop);
        }
    }

    {
        using namespace KisStandardUniformPropertiesFactory;

        Q_FOREACH (KisUniformPaintOpPropertySP prop, KisPaintOpSettings::uniformProperties(settings)) {
            if (prop->id() == opacity.id()) {
                props.prepend(prop);
            }
        }
    }

    return props;
}
