/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisOpacityOption.h"

#include <kis_painter.h>

quint8 KisOpacityOption::apply(KisPainter* painter, const KisPaintInformation& info) const
{
    if (!isChecked()) {
        return painter->opacity();
    }
    quint8 origOpacity = painter->opacity();

    qreal opacity = (qreal)(origOpacity * computeSizeLikeValue(info));
    quint8 opacity2 = (quint8)qRound(qBound<qreal>(OPACITY_TRANSPARENT_U8, opacity, OPACITY_OPAQUE_U8));

    painter->setOpacityUpdateAverage(opacity2);
    return origOpacity;
}
