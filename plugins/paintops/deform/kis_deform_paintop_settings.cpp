/*
 *  SPDX-FileCopyrightText: 2008, 2009, 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <kis_deform_paintop_settings.h>
#include <kis_deform_paintop_settings_widget.h>

#include <kis_brush_size_option.h>

#include <kis_airbrush_option_widget.h>
#include <kis_deform_option.h>

struct KisDeformPaintOpSettings::Private
{
    QList<KisUniformPaintOpPropertyWSP> uniformProperties;
};

KisDeformPaintOpSettings::KisDeformPaintOpSettings(KisResourcesInterfaceSP resourcesInterface)
    : KisOutlineGenerationPolicy<KisPaintOpSettings>(KisCurrentOutlineFetcher::SIZE_OPTION |
                                                     KisCurrentOutlineFetcher::ROTATION_OPTION,
                                                     resourcesInterface),
    m_d(new Private)
{
}

KisDeformPaintOpSettings::~KisDeformPaintOpSettings()
{
}

void KisDeformPaintOpSettings::setPaintOpSize(qreal value)
{
    KisBrushSizeOptionProperties option;
    option.readOptionSetting(this);
    option.brush_diameter = value;
    option.writeOptionSetting(this);
}

qreal KisDeformPaintOpSettings::paintOpSize() const
{
    KisBrushSizeOptionProperties option;
    option.readOptionSetting(this);
    return option.brush_diameter;
}

bool KisDeformPaintOpSettings::paintIncremental()
{
    return true;
}

bool KisDeformPaintOpSettings::isAirbrushing() const
{
    // version 2.3
    if (hasProperty(AIRBRUSH_ENABLED)) {
        return getBool(AIRBRUSH_ENABLED);
    }
    else {
        return getBool(DEFORM_USE_MOVEMENT_PAINT);
    }
}

QPainterPath KisDeformPaintOpSettings::brushOutline(const KisPaintInformation &info, const OutlineMode &mode, qreal alignForZoom)
{
    QPainterPath path;
    if (mode.isVisible) {
        qreal width = getInt(BRUSH_DIAMETER);
        qreal height = getInt(BRUSH_DIAMETER) * getDouble(BRUSH_ASPECT);
        path = ellipseOutline(width, height, getDouble(BRUSH_SCALE), getDouble(BRUSH_ROTATION));

        path = outlineFetcher()->fetchOutline(info, this, path, mode, alignForZoom);

        if (mode.showTiltDecoration) {
            QPainterPath tiltLine = makeTiltIndicator(info, QPointF(0.0, 0.0), width * 0.5, 3.0);
            path.addPath(outlineFetcher()->fetchOutline(info, this, tiltLine, mode, alignForZoom, 1.0, 0.0, true, 0, 0));
        }
    }
    return path;
}


#include <brushengine/kis_slider_based_paintop_property.h>
#include <brushengine/kis_combo_based_paintop_property.h>
#include "kis_paintop_preset.h"
#include "kis_paintop_settings_update_proxy.h"
#include "kis_standard_uniform_properties_factory.h"


QList<KisUniformPaintOpPropertySP> KisDeformPaintOpSettings::uniformProperties(KisPaintOpSettingsSP settings)
{
    QList<KisUniformPaintOpPropertySP> props =
        listWeakToStrong(m_d->uniformProperties);

    if (props.isEmpty()) {
        {
            KisDoubleSliderBasedPaintOpPropertyCallback *prop =
                new KisDoubleSliderBasedPaintOpPropertyCallback(
                    KisDoubleSliderBasedPaintOpPropertyCallback::Double,
                    "deform_amount",
                    i18n("Amount"),
                    settings, 0);

            prop->setRange(0.01, 1.0);
            prop->setSingleStep(0.01);
            prop->setDecimals(2);

            prop->setReadCallback(
                [](KisUniformPaintOpProperty *prop) {
                    DeformOption option;
                    option.readOptionSetting(prop->settings().data());
                    prop->setValue(option.deform_amount);
                });
            prop->setWriteCallback(
                [](KisUniformPaintOpProperty *prop) {
                    DeformOption option;
                    option.readOptionSetting(prop->settings().data());
                    option.deform_amount = prop->value().toReal();
                    option.writeOptionSetting(prop->settings().data());
                });

            QObject::connect(updateProxy(), SIGNAL(sigSettingsChanged()), prop, SLOT(requestReadValue()));
            prop->requestReadValue();
            props << toQShared(prop);
        }

        {
            KisComboBasedPaintOpPropertyCallback *prop =
                new KisComboBasedPaintOpPropertyCallback(
                    "deform_mode",
                    i18n("Deform Mode"),
                    settings, 0);

            QList<QString> modes;
            modes << i18n("Grow");
            modes << i18n("Shrink");
            modes << i18n("Swirl CW");
            modes << i18n("Swirl CCW");
            modes << i18n("Move");
            modes << i18n("Lens Zoom In");
            modes << i18n("Lens Zoom Out");
            modes << i18n("Color Deformation");

            prop->setItems(modes);

            prop->setReadCallback(
                [](KisUniformPaintOpProperty *prop) {
                    DeformOption option;
                    option.readOptionSetting(prop->settings().data());
                    prop->setValue(int(option.deform_action - 1));
                });
            prop->setWriteCallback(
                [](KisUniformPaintOpProperty *prop) {
                    DeformOption option;
                    option.readOptionSetting(prop->settings().data());
                    option.deform_action = prop->value().toInt() + 1;
                    option.writeOptionSetting(prop->settings().data());
                });

            QObject::connect(updateProxy(), SIGNAL(sigSettingsChanged()), prop, SLOT(requestReadValue()));
            prop->requestReadValue();
            props << toQShared(prop);
        }


        {
            KisIntSliderBasedPaintOpPropertyCallback *prop =
                new KisIntSliderBasedPaintOpPropertyCallback(
                    KisIntSliderBasedPaintOpPropertyCallback::Int,
                    "deform_angle",
                    i18n("Angle"),
                    settings, 0);

            const QString degree = QChar(Qt::Key_degree);
            prop->setRange(0, 360);
            prop->setSingleStep(1);
            prop->setSuffix(degree);

            prop->setReadCallback(
                [](KisUniformPaintOpProperty *prop) {
                    KisBrushSizeOptionProperties option;
                    option.readOptionSetting(prop->settings().data());

                    prop->setValue(int(option.brush_rotation));
                });
            prop->setWriteCallback(
                [](KisUniformPaintOpProperty *prop) {
                    KisBrushSizeOptionProperties option;
                    option.readOptionSetting(prop->settings().data());
                    option.brush_rotation = prop->value().toInt();
                    option.writeOptionSetting(prop->settings().data());
                });

            QObject::connect(updateProxy(), SIGNAL(sigSettingsChanged()), prop, SLOT(requestReadValue()));
            prop->requestReadValue();
            props << toQShared(prop);
        }
    }

    {
        using namespace KisStandardUniformPropertiesFactory;

        Q_FOREACH (KisUniformPaintOpPropertySP prop, KisPaintOpSettings::uniformProperties(settings)) {
            if (prop->id() == opacity.id() ||
                prop->id() == size.id()) {
                props.prepend(prop);
            }
        }
    }

    return props;
}
