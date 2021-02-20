/*
 *  SPDX-FileCopyrightText: 2016 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_roundmarkerop_settings.h"
#include "kis_roundmarker_option.h"


struct KisRoundMarkerOpSettings::Private
{
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
    RoundMarkerOption op;
    op.readOptionSetting(*this);
    op.diameter = value;
    op.writeOptionSetting(this);
}

qreal KisRoundMarkerOpSettings::paintOpSize() const
{
    RoundMarkerOption op;
    op.readOptionSetting(*this);
    return op.diameter;
}

QPainterPath KisRoundMarkerOpSettings::brushOutline(const KisPaintInformation &info, const OutlineMode &mode, qreal alignForZoom)
{
    QPainterPath path;

    if (mode.isVisible) {
        qreal finalScale = 1.0;

        RoundMarkerOption op;
        op.readOptionSetting(*this);
        // Adding 1 for the antialiasing/fade.
        const qreal radius = (0.5 * op.diameter) + 1;

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

QList<KisUniformPaintOpPropertySP> KisRoundMarkerOpSettings::uniformProperties(KisPaintOpSettingsSP settings)
{
    QList<KisUniformPaintOpPropertySP> props;

    {
        using namespace KisStandardUniformPropertiesFactory;

        Q_FOREACH (KisUniformPaintOpPropertySP prop, KisPaintOpSettings::uniformProperties(settings)) {
            if (prop->id() != flow.id()) {
                props.prepend(prop);
            }
        }
    }

    return props;
}
