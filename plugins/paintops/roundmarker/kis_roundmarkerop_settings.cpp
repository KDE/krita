/*
 *  Copyright (c) 2016 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_roundmarkerop_settings.h"
#include "kis_roundmarker_option.h"


struct KisRoundMarkerOpSettings::Private
{
};

KisRoundMarkerOpSettings::KisRoundMarkerOpSettings()
    : KisOutlineGenerationPolicy<KisPaintOpSettings>(KisCurrentOutlineFetcher::SIZE_OPTION |
                                                     KisCurrentOutlineFetcher::ROTATION_OPTION |
                                                     KisCurrentOutlineFetcher::MIRROR_OPTION),
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
    op.diameter = qBound(0.01, value, 1000.0);
    op.writeOptionSetting(this);
}

qreal KisRoundMarkerOpSettings::paintOpSize() const
{
    RoundMarkerOption op;
    op.readOptionSetting(*this);
    return op.diameter;
}

QPainterPath KisRoundMarkerOpSettings::brushOutline(const KisPaintInformation &info, const OutlineMode &mode)
{
    QPainterPath path;

    if (mode.isVisible) {
        qreal finalScale = 1.0;

        RoundMarkerOption op;
        op.readOptionSetting(*this);
        const qreal radius = 0.5 * op.diameter;

        QPainterPath realOutline;
        realOutline.addEllipse(QPointF(), radius, radius);

        path = outlineFetcher()->fetchOutline(info, this, realOutline, mode, finalScale);

        if (mode.showTiltDecoration) {
            QPainterPath tiltLine = makeTiltIndicator(info,
                realOutline.boundingRect().center(),
                realOutline.boundingRect().width() * 0.5,
                3.0);
            path.addPath(outlineFetcher()->fetchOutline(info, this, tiltLine, mode, finalScale, 0.0, true, realOutline.boundingRect().center().x(), realOutline.boundingRect().center().y()));
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
