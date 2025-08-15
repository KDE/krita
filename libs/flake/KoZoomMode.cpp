/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2005 Johannes Schaub <litb_devel@web.de>
   SPDX-FileCopyrightText: 2011 Arjen Hiemstra <ahiemstra@heimr.nl>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "KoZoomMode.h"

#include <QDebug>

#include <klocalizedstring.h>
#include <kconfiggroup.h>
#include <ksharedconfig.h>

#include <cmath>
#include "kis_assert.h"

QString KoZoomMode::toString(Mode mode)
{
    switch (mode) {
    case KoZoomMode::ZOOM_CONSTANT:
        KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(0 && "constant zoom has no user-friendly representation", "XXX");
    case KoZoomMode::ZOOM_PAGE:
        return i18n("Fit View");
    case KoZoomMode::ZOOM_WIDTH:
        return i18n("Fit Width");
    case KoZoomMode::ZOOM_HEIGHT:
        return i18n("Fit Height");
    }
    return "";
}

QVector<qreal> KoZoomMode::generateStandardZoomLevels(qreal minZoom, qreal maxZoom)
{
    KConfigGroup config = KSharedConfig::openConfig()->group("");
    int steps = config.readEntry("zoomSteps", 2);
    qreal k = steps / M_LN2;

    int first =  ceil(log(minZoom) * k);
    int size  = floor(log(maxZoom) * k) - first + 1;
    QVector<qreal> zoomLevels(size);

    // enforce zoom levels relating to thirds (33.33%, 66.67%, ...)
    QVector<qreal> snap(steps);
    if (steps > 1) {
        qreal third = log(4./ 3.) * k;
        int i = round(third);
        snap[(i - first) % steps] = third - i;
    }

    k = 1./ k;
    for (int i = 0; i < steps; i++) {
        qreal f = exp((i + first + snap[i]) * k);
        f = floor(f * 0x1p48 + 0.5) / 0x1p48; // round off inaccuracies
        for (int j = i; j < size; j += steps, f *= 2.) {
            zoomLevels[j] = f;
        }
    }
    return zoomLevels;
}

qreal KoZoomMode::findNextZoom(qreal currentZoom, const QVector<qreal> &zoomLevels)
{
    const qreal eps = 1e-5;
    int i = 0;
    while (i < zoomLevels.size() - 1 && currentZoom > zoomLevels[i] - eps) {
        i++;
    }

    return qMax(currentZoom, zoomLevels[i]);
}

qreal KoZoomMode::findPrevZoom(qreal currentZoom, const QVector<qreal> &zoomLevels)
{
    const qreal eps = 1e-5;
    int i = zoomLevels.size() - 1;
    while (i > 0 && currentZoom < zoomLevels[i] + eps) i--;

    return qMin(currentZoom, zoomLevels[i]);
}

QDebug operator<<(QDebug dbg, const KoZoomMode::Mode &mode)
{
    dbg.nospace() << "KoZoomMode::Mode(";

    switch (mode) {
    case KoZoomMode::ZOOM_CONSTANT:
        dbg << "ZOOM_CONSTANT";
        break;
    case KoZoomMode::ZOOM_PAGE:
        dbg << "ZOOM_PAGE";
        break;
    case KoZoomMode::ZOOM_WIDTH:
        dbg << "ZOOM_WIDTH";
        break;
    case KoZoomMode::ZOOM_HEIGHT:
        dbg << "ZOOM_HEIGHT";
        break;
    default:
        dbg << "UNKNOWN";
        break;
    }

    dbg << ")";
    return dbg.space();
}
