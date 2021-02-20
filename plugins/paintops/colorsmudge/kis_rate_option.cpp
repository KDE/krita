/* This file is part of the KDE project
 *
 * SPDX-FileCopyrightText: 2011 Silvio Heinrich <plassy@web.de>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "kis_rate_option.h"

#include <klocalizedstring.h>

#include <kis_painter.h>
#include <widgets/kis_curve_widget.h>

#include <KoColor.h>

#include <iostream>

KisRateOption::KisRateOption(const QString& name, KisPaintOpOption::PaintopCategory category, bool checked):
    KisCurveOption(name, category, checked)
{
}

void KisRateOption::apply(KisPainter& painter, const KisPaintInformation& info, qreal scaleMin, qreal scaleMax, qreal multiplicator) const
{
    if (!isChecked()) {
        painter.setOpacity((quint8)(scaleMax * 255.0));
        return;
    }

    qreal value = computeSizeLikeValue(info);

    qreal  rate    = scaleMin + (scaleMax - scaleMin) * multiplicator * value; // scale m_rate into the range scaleMin - scaleMax
    quint8 opacity = qBound(OPACITY_TRANSPARENT_U8, (quint8)(rate * 255.0), OPACITY_OPAQUE_U8);
    painter.setOpacity(opacity);
}
