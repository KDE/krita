/*
 *  SPDX-FileCopyrightText: 2008, 2009, 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <KisOptimizedBrushOutline.h>
#include <kis_deform_paintop_settings.h>
#include <kis_deform_paintop_settings_widget.h>

#include "KisBrushSizeOptionData.h"

#include "KisDeformOptionData.h"

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
    KisBrushSizeOptionData option;
    option.read(this);
    option.brushDiameter = value;
    option.write(this);
}

qreal KisDeformPaintOpSettings::paintOpSize() const
{
    KisBrushSizeOptionData option;
    option.read(this);
    return option.brushDiameter;
}

void KisDeformPaintOpSettings::setPaintOpAngle(qreal value)
{
    Q_UNUSED(value);
}

qreal KisDeformPaintOpSettings::paintOpAngle() const
{
    return 0.0;
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
        return getBool("Deform/useMovementPaint");
    }
}

KisOptimizedBrushOutline KisDeformPaintOpSettings::brushOutline(const KisPaintInformation &info, const OutlineMode &mode, qreal alignForZoom)
{
    KisOptimizedBrushOutline path;
    if (mode.isVisible) {
        qreal width = getInt("Brush/diameter");
        qreal height = getInt("Brush/diameter") * getDouble("Brush/aspect");
        path = ellipseOutline(width, height, getDouble("Brush/scale"), -getDouble("Brush/rotation"));

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
#include "KisPaintOpPresetUpdateProxy.h"
#include "kis_standard_uniform_properties_factory.h"


QList<KisUniformPaintOpPropertySP> KisDeformPaintOpSettings::uniformProperties(KisPaintOpSettingsSP settings, QPointer<KisPaintOpPresetUpdateProxy> updateProxy)
{
    QList<KisUniformPaintOpPropertySP> props =
        listWeakToStrong(m_d->uniformProperties);

    if (props.isEmpty()) {
        {
            KisDoubleSliderBasedPaintOpPropertyCallback *prop =
                new KisDoubleSliderBasedPaintOpPropertyCallback(KisDoubleSliderBasedPaintOpPropertyCallback::Double,
                                                                KoID("deform_amount", i18n("Amount")),
                                                                settings,
                                                                0);

            prop->setRange(0.01, 1.0);
            prop->setSingleStep(0.01);
            prop->setDecimals(2);

            prop->setReadCallback(
                [](KisUniformPaintOpProperty *prop) {
                    KisDeformOptionData option;
                    option.read(prop->settings().data());
                    prop->setValue(option.deformAmount);
                });
            prop->setWriteCallback(
                [](KisUniformPaintOpProperty *prop) {
                    KisDeformOptionData option;
                    option.read(prop->settings().data());
                    option.deformAmount = prop->value().toReal();
                    option.write(prop->settings().data());
                });

            QObject::connect(updateProxy, SIGNAL(sigSettingsChanged()), prop, SLOT(requestReadValue()));
            prop->requestReadValue();
            props << toQShared(prop);
        }

        {
            KisComboBasedPaintOpPropertyCallback *prop = new KisComboBasedPaintOpPropertyCallback(KoID("deform_mode", i18n("Deform Mode")), settings, 0);

            QList<QString> modes;
            modes << i18nc("Grow as in deform brush engine", "Grow");
            modes << i18nc("Shrink as in deform brush engine", "Shrink");
            modes << i18n("Swirl CW");
            modes << i18n("Swirl CCW");
            modes << i18n("Move");
            modes << i18n("Lens Zoom In");
            modes << i18n("Lens Zoom Out");
            modes << i18nc("Rearrange the positions of the pixels under the cursor", "Color Deformation");

            prop->setItems(modes);

            prop->setReadCallback(
                [](KisUniformPaintOpProperty *prop) {
                    KisDeformOptionData option;
                    option.read(prop->settings().data());
                    prop->setValue(int(option.deformAction) - 1); // DeformModes start at 1, combobox indices start at 0
                });
            prop->setWriteCallback(
                [](KisUniformPaintOpProperty *prop) {
                    KisDeformOptionData option;
                    option.read(prop->settings().data());
                    option.deformAction = (DeformModes)(prop->value().toInt() + 1); // Combobox indices start at 0, DeformModes start at 1
                    option.write(prop->settings().data());
                });

            QObject::connect(updateProxy, SIGNAL(sigSettingsChanged()), prop, SLOT(requestReadValue()));
            prop->requestReadValue();
            props << toQShared(prop);
        }


        {
            KisIntSliderBasedPaintOpPropertyCallback *prop =
                new KisIntSliderBasedPaintOpPropertyCallback(KisIntSliderBasedPaintOpPropertyCallback::Int,
                                                             KisIntSliderBasedPaintOpPropertyCallback::SubType_Angle,
                                                             KoID("deform_angle", i18n("Angle")),
                                                             settings,
                                                             0);

            const QString degree = QChar(Qt::Key_degree);
            prop->setRange(0, 360);
            prop->setSingleStep(1);
            prop->setSuffix(degree);

            prop->setReadCallback(
                [](KisUniformPaintOpProperty *prop) {
                    KisBrushSizeOptionData option;
                    option.read(prop->settings().data());

                    prop->setValue(int(option.brushRotation));
                });
            prop->setWriteCallback(
                [](KisUniformPaintOpProperty *prop) {
                    KisBrushSizeOptionData option;
                    option.read(prop->settings().data());
                    option.brushRotation = prop->value().toInt();
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
            if (prop->id() == opacity.id() ||
                prop->id() == size.id()) {
                props.prepend(prop);
            }
        }
    }

    return props;
}
