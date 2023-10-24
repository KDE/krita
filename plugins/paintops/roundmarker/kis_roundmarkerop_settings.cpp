/*
 *  SPDX-FileCopyrightText: 2016 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_roundmarkerop_settings.h"
#include "KisRoundMarkerOpOptionData.h"
#include <KisOptimizedBrushOutline.h>
#include <QPointer>


struct KisRoundMarkerOpSettings::Private
{
    QList<KisUniformPaintOpPropertyWSP> uniformProperties;
};

KisRoundMarkerOpSettings::KisRoundMarkerOpSettings(KisResourcesInterfaceSP resourcesInterface)
    : KisOutlineGenerationPolicy<KisPaintOpSettings>(KisCurrentOutlineFetcher::SIZE_OPTION |
                                                     KisCurrentOutlineFetcher::ROTATION_OPTION |
                                                     KisCurrentOutlineFetcher::MIRROR_OPTION,
                                                     resourcesInterface),
    m_d(new Private)
{
}

KisRoundMarkerOpSettings::~KisRoundMarkerOpSettings()
{
}

bool KisRoundMarkerOpSettings::paintIncremental() {
    return false;
}

void KisRoundMarkerOpSettings::setPaintOpSize(qreal value)
{
    KisRoundMarkerOpOptionData option;
    option.read(this);
    option.diameter = value;
    option.write(this);
}

qreal KisRoundMarkerOpSettings::paintOpSize() const
{
    KisRoundMarkerOpOptionData option;
    option.read(this);
    return option.diameter;
}

void KisRoundMarkerOpSettings::setPaintOpAngle(qreal value)
{
    Q_UNUSED(value);
}

qreal KisRoundMarkerOpSettings::paintOpAngle() const
{
    return 0.0;
}

KisOptimizedBrushOutline KisRoundMarkerOpSettings::brushOutline(const KisPaintInformation &info, const OutlineMode &mode, qreal alignForZoom)
{
    KisOptimizedBrushOutline path;

    if (mode.isVisible) {
        qreal finalScale = 1.0;

        KisRoundMarkerOpOptionData option;
        option.read(this);
        // Adding 1 for the antialiasing/fade.
        const qreal radius = (0.5 * option.diameter) + 1;

        QPainterPath realOutline;
        realOutline.addEllipse(QPointF(), radius, radius);

        path = outlineFetcher()->fetchOutline(info, this, realOutline, mode, alignForZoom, finalScale);

        if (mode.showTiltDecoration) {
            QPainterPath tiltLine = makeTiltIndicator(info,
                realOutline.boundingRect().center(),
                realOutline.boundingRect().width() * 0.5,
                3.0);
            path.addPath(outlineFetcher()->fetchOutline(info, this, tiltLine, mode, alignForZoom, finalScale, 0.0, true, realOutline.boundingRect().center().x(), realOutline.boundingRect().center().y()));
        }
    }

    return path;
}

#include "kis_standard_uniform_properties_factory.h"
#include <brushengine/kis_slider_based_paintop_property.h>
#include "KisPaintOpPresetUpdateProxy.h"

QList<KisUniformPaintOpPropertySP> KisRoundMarkerOpSettings::uniformProperties(KisPaintOpSettingsSP settings, QPointer<KisPaintOpPresetUpdateProxy> updateProxy)
{
    QList<KisUniformPaintOpPropertySP> props =
        listWeakToStrong(m_d->uniformProperties);

    if (props.isEmpty()) {
        {
            KisUniformPaintOpPropertyCallback *prop =
                new KisUniformPaintOpPropertyCallback(KisUniformPaintOpPropertyCallback::Bool, KoID("auto_spacing", i18n("Auto Spacing")), settings, 0);

            prop->setReadCallback(
                [](KisUniformPaintOpProperty *prop) {
                    KisRoundMarkerOpOptionData data;
                    data.read(prop->settings().data());

                    prop->setValue(data.useAutoSpacing);
                });
            prop->setWriteCallback(
                [](KisUniformPaintOpProperty *prop) {
                    KisRoundMarkerOpOptionData data;
                    data.read(prop->settings().data());
                    data.useAutoSpacing = prop->value().toBool();
                    data.write(prop->settings().data());
                });

            QObject::connect(updateProxy, SIGNAL(sigSettingsChanged()), prop, SLOT(requestReadValue()));
            prop->requestReadValue();
            props << toQShared(prop);
        }

        {
            KisDoubleSliderBasedPaintOpPropertyCallback *prop =
                new KisDoubleSliderBasedPaintOpPropertyCallback(KisDoubleSliderBasedPaintOpPropertyCallback::Double,
                                                                KoID("spacing", i18n("Spacing")),
                                                                settings,
                                                                0);

            prop->setRange(0.01, 10);
            prop->setSingleStep(0.01);
            prop->setExponentRatio(3.0);

            prop->setReadCallback(
                [](KisUniformPaintOpProperty *prop) {
                    KisRoundMarkerOpOptionData data;
                    data.read(prop->settings().data());

                    prop->setValue(data.autoSpacingCoeff);
                });
            prop->setWriteCallback(
                [](KisUniformPaintOpProperty *prop) {
                    KisRoundMarkerOpOptionData data;
                    data.read(prop->settings().data());
                    data.autoSpacingCoeff = prop->value().toBool();
                    data.write(prop->settings().data());
                });

            QObject::connect(updateProxy, SIGNAL(sigSettingsChanged()), prop, SLOT(requestReadValue()));
            prop->requestReadValue();
            props << toQShared(prop);
        }
    }

    return KisPaintOpSettings::uniformProperties(settings, updateProxy) + props;
}
