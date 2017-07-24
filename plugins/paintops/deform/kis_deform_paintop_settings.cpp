/*
 *  Copyright (c) 2008,2009,2010 Lukáš Tvrdý <lukast.dev@gmail.com>
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

#include <kis_deform_paintop_settings.h>
#include <kis_deform_paintop_settings_widget.h>

#include <kis_brush_size_option.h>

#include <kis_airbrush_option.h>
#include <kis_deform_option.h>

struct KisDeformPaintOpSettings::Private
{
    QList<KisUniformPaintOpPropertyWSP> uniformProperties;
};

KisDeformPaintOpSettings::KisDeformPaintOpSettings()
    : KisOutlineGenerationPolicy<KisPaintOpSettings>(KisCurrentOutlineFetcher::SIZE_OPTION |
                                                     KisCurrentOutlineFetcher::ROTATION_OPTION),
    m_d(new Private)
{
}

KisDeformPaintOpSettings::~KisDeformPaintOpSettings()
{
}

void KisDeformPaintOpSettings::setPaintOpSize(qreal value)
{
    BrushSizeOption option;
    option.readOptionSetting(this);
    option.brush_diameter = value;
    option.writeOptionSetting(this);
}

qreal KisDeformPaintOpSettings::paintOpSize() const
{
    BrushSizeOption option;
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

QPainterPath KisDeformPaintOpSettings::brushOutline(const KisPaintInformation &info, OutlineMode mode)
{
    QPainterPath path;
    if (mode == CursorIsOutline || mode == CursorIsCircleOutline || mode == CursorTiltOutline) {
        qreal width = getInt(BRUSH_DIAMETER);
        qreal height = getInt(BRUSH_DIAMETER) * getDouble(BRUSH_ASPECT);
        path = ellipseOutline(width, height, getDouble(BRUSH_SCALE), getDouble(BRUSH_ROTATION));

        QPainterPath tiltLine;
        QLineF tiltAngle(QPointF(0.0,0.0), QPointF(0.0,width));
        tiltAngle.setLength(qMax(width*0.5, 50.0) * (1 - info.tiltElevation(info, 60.0, 60.0, true)));
        tiltAngle.setAngle((360.0 - fmod(KisPaintInformation::tiltDirection(info, true) * 360.0 + 270.0, 360.0))-3.0);
        tiltLine.moveTo(tiltAngle.p1());
        tiltLine.lineTo(tiltAngle.p2());
        tiltAngle.setAngle((360.0 - fmod(KisPaintInformation::tiltDirection(info, true) * 360.0 + 270.0, 360.0))+3.0);
        tiltLine.lineTo(tiltAngle.p2());
        tiltLine.lineTo(tiltAngle.p1());


        path = outlineFetcher()->fetchOutline(info, this, path);

        if (mode == CursorTiltOutline) {
            QPainterPath tiltLine = makeTiltIndicator(info, QPointF(0.0, 0.0), width * 0.5, 3.0);
            path.addPath(outlineFetcher()->fetchOutline(info, this, tiltLine, 1.0, 0.0, true, 0, 0));
        }
    }
    return path;
}


#include <brushengine/kis_slider_based_paintop_property.h>
#include <brushengine/kis_combo_based_paintop_property.h>
#include "kis_paintop_preset.h"
#include "kis_paintop_settings_update_proxy.h"
#include "kis_brush_size_option.h"
#include "kis_deform_option.h"
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

            QObject::connect(preset()->updateProxy(), SIGNAL(sigSettingsChanged()), prop, SLOT(requestReadValue()));
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

            QObject::connect(preset()->updateProxy(), SIGNAL(sigSettingsChanged()), prop, SLOT(requestReadValue()));
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
                    BrushSizeOption option;
                    option.readOptionSetting(prop->settings().data());

                    prop->setValue(int(option.brush_rotation));
                });
            prop->setWriteCallback(
                [](KisUniformPaintOpProperty *prop) {
                    BrushSizeOption option;
                    option.readOptionSetting(prop->settings().data());
                    option.brush_rotation = prop->value().toInt();
                    option.writeOptionSetting(prop->settings().data());
                });

            QObject::connect(preset()->updateProxy(), SIGNAL(sigSettingsChanged()), prop, SLOT(requestReadValue()));
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
