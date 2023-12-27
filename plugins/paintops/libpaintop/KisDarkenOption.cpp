/*
 *  SPDX-FileCopyrightText: 2008 Boudewijn Rempt <boud@valdyas.org>
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisDarkenOption.h"

#include <KisPaintOpOptionUtils.h>
#include <KisStandardOptionData.h>
#include <kis_paint_device.h>
#include <kis_paint_information.h>
#include <kis_painter.h>
#include <kis_properties_configuration.h>

#include "kis_color_source.h"

namespace kpou = KisPaintOpOptionUtils;


KisDarkenOption::KisDarkenOption(const KisPropertiesConfiguration *setting)
    : KisCurveOption(kpou::loadOptionData<KisDarkenOptionData>(setting))
{
}

KoColor KisDarkenOption::apply(KisPainter * painter, const KisPaintInformation& info) const
{
    if (!isChecked()) {
        return painter->paintColor();
    }

    KoColor darkened = painter->paintColor();
    KoColor origColor = darkened;

    // Darken docs aren't really clear about what exactly the amount param can have as value...
    quint32 darkenAmount = (qint32)(255  - 255 * computeSizeLikeValue(info));

    KoColorTransformation* darkenTransformation  = darkened.colorSpace()->createDarkenAdjustment(darkenAmount, false, 0.0);
    if (!darkenTransformation) return origColor;
    darkenTransformation ->transform(painter->paintColor().data(), darkened.data(), 1);
    painter->setPaintColor(darkened);
    delete darkenTransformation;

    return origColor;
}

void KisDarkenOption::apply(KisColorSource* colorSource, const KisPaintInformation& info) const
{
    if (!isChecked()) return;

    // Darken docs aren't really clear about what exactly the amount param can have as value...
    quint32 darkenAmount = (qint32)(255  - 255 * computeSizeLikeValue(info));

    KoColorTransformation* darkenTransformation  = colorSource->colorSpace()->createDarkenAdjustment(darkenAmount, false, 0.0);
    if (!darkenTransformation) return;
    colorSource->applyColorTransformation(darkenTransformation);

    delete darkenTransformation;
}
