/*
 *  SPDX-FileCopyrightText: 2009, 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_experiment_paintop_settings.h"
#include "kis_current_outline_fetcher.h"
#include "kis_algebra_2d.h"

struct KisExperimentPaintOpSettings::Private
{
    QList<KisUniformPaintOpPropertyWSP> uniformProperties;
};

KisExperimentPaintOpSettings::KisExperimentPaintOpSettings(KisResourcesInterfaceSP resourcesInterface)
    : KisNoSizePaintOpSettings(resourcesInterface),
      m_d(new Private)
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

QPainterPath KisExperimentPaintOpSettings::brushOutline(const KisPaintInformation &info, const OutlineMode &mode, qreal alignForZoom)
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

        path.translate(KisAlgebra2D::alignForZoom(info.pos(), alignForZoom));
    }
    return path;
}

#include <brushengine/kis_slider_based_paintop_property.h>
#include "kis_paintop_preset.h"
#include "KisPaintOpPresetUpdateProxy.h"
#include "kis_experimentop_option.h"
#include "kis_standard_uniform_properties_factory.h"


QList<KisUniformPaintOpPropertySP> KisExperimentPaintOpSettings::uniformProperties(KisPaintOpSettingsSP settings, QPointer<KisPaintOpPresetUpdateProxy> updateProxy)
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

            QObject::connect(updateProxy, SIGNAL(sigSettingsChanged()), prop, SLOT(requestReadValue()));
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

            QObject::connect(updateProxy, SIGNAL(sigSettingsChanged()), prop, SLOT(requestReadValue()));
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

            QObject::connect(updateProxy, SIGNAL(sigSettingsChanged()), prop, SLOT(requestReadValue()));
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

            QObject::connect(updateProxy, SIGNAL(sigSettingsChanged()), prop, SLOT(requestReadValue()));
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
