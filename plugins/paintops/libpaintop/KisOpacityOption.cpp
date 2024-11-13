/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisOpacityOption.h"

#include <kis_painter.h>

qreal KisOpacityOption::apply(KisPainter* painter, const KisPaintInformation& info) const
{
    if (!isChecked()) {
        return painter->opacityF();
    }
    qreal origOpacity = painter->opacityF();

    qreal opacity = origOpacity * computeSizeLikeValue(info);
    qreal opacity2 = qBound<qreal>(OPACITY_TRANSPARENT_F, opacity, OPACITY_OPAQUE_F);

    painter->setOpacityUpdateAverage(opacity2);
    return origOpacity;
}
