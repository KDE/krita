/*
 *  SPDX-FileCopyrightText: 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisSharpnessOption.h"

#include <kis_properties_configuration.h>
#include <kis_paintop.h>
#include <kis_fixed_paint_device.h>

#include <KisSharpnessOptionData.h>

#include <KisPaintOpOptionUtils.h>
namespace kpou = KisPaintOpOptionUtils;


KisSharpnessOption::KisSharpnessOption(const KisPropertiesConfiguration *setting)
    : KisSharpnessOption(kpou::loadOptionData<KisSharpnessOptionData>(setting))
{
}

KisSharpnessOption::KisSharpnessOption(const KisSharpnessOptionData &data)
    : KisCurveOption(data)
    , m_alignOutlinePixels(data.alignOutlinePixels)
    , m_softness(data.softness)
{

}

void KisSharpnessOption::apply(const KisPaintInformation &info, const QPointF &pt, qint32 &x, qint32 &y, qreal &xFraction, qreal &yFraction) const
{
    if (isChecked() && m_alignOutlinePixels && strengthValue() > 0.0) {
        qreal processedSharpness = computeSizeLikeValue(info);

        if (qFuzzyCompare(processedSharpness, 1.0)) {
            // pen
            xFraction = 0.0;
            yFraction = 0.0;
            x = qRound(pt.x());
            y = qRound(pt.y());
        }
        else {
            // something in between
            qint32 xi = qRound(pt.x());
            qint32 yi = qRound(pt.y());

            qreal xf = processedSharpness * xi + (1.0 - processedSharpness) * pt.x();
            qreal yf = processedSharpness * yi + (1.0 - processedSharpness) * pt.y();

            KisPaintOp::splitCoordinate(xf, &x, &xFraction);
            KisPaintOp::splitCoordinate(yf, &y, &yFraction);
        }
    } else {
        // brush
        KisPaintOp::splitCoordinate(pt.x(), &x, &xFraction);
        KisPaintOp::splitCoordinate(pt.y(), &y, &yFraction);
    }
}

void KisSharpnessOption::applyThreshold(KisFixedPaintDeviceSP dab, const KisPaintInformation &info)
{
    if (!isChecked()) return;
    const KoColorSpace * cs = dab->colorSpace();

    // Set all alpha > opaque/2 to opaque, the rest to transparent.
    // XXX: Using 4/10 as the 1x1 circle brush paints nothing with 0.5.
    quint8* dabPointer = dab->data();
    QRect rc = dab->bounds();

    qreal threshold = computeSizeLikeValue(info);

    quint32 pixelSize = dab->pixelSize();
    int pixelCount = rc.width() * rc.height();

    quint32 tolerance = quint32(OPACITY_OPAQUE_U8 - (threshold * OPACITY_OPAQUE_U8));

    for (int i = 0; i < pixelCount; i++) {
        quint8 opacity = cs->opacityU8(dabPointer);

        // Check what pixel goes sharp
        if (opacity > (tolerance) ) {
            cs->setOpacity(dabPointer, OPACITY_OPAQUE_U8, 1);
        } else {
            // keep original value if in soft range
            if (opacity <= (100 - m_softness) * tolerance / 100) {
                cs->setOpacity(dabPointer, OPACITY_TRANSPARENT_U8, 1);
            }
        }
        dabPointer += pixelSize;
    }
}

bool KisSharpnessOption::alignOutlineToPixels() const
{
    return m_alignOutlinePixels;
}
