/*
 *  Copyright (c) 2004 Michael Thaler <michael.thaler@physik.tu-muenchen.de>
 *  Copyright (c) 2005 C. Boemann <cbo@boemann.dk>
 *  Copyright (c) 2013 Juan Palacios <jpalaciosdev@gmail.com>
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

#include "kis_filter_strategy.h"

#include <math.h>

#include <klocalizedstring.h>
#include <QGlobalStatic>

#include "kis_debug.h"
#include <QtMath>

Q_GLOBAL_STATIC(KisFilterStrategyRegistry, s_instance)

qreal KisHermiteFilterStrategy::valueAt(qreal t, qreal weightsPositionScale) const
{
    Q_UNUSED(weightsPositionScale);
    /* f(t) = 2|t|^3 - 3|t|^2 + 1, -1 <= t <= 1 */
    if (t < 0.0) t = -t;
    if (t < 1.0) return((2.0 * t - 3.0) * t * t + 1.0);
    return(0.0);
}

qint32 KisHermiteFilterStrategy::intValueAt(qint32 t, qreal weightsPositionScale) const
{
    Q_UNUSED(weightsPositionScale);
    /* f(t) = 2|t|^3 - 3|t|^2 + 1, -1 <= t <= 1 */
    if (t < 0) t = -t;
    if (t < 256) {
        t = (2 * t - 3 * 256) * t * t + (256 << 16);

        //go from .24 fixed point to .8 fixedpoint (hack only works with positive numbers, which it is)
        t = (t + 0x8000) >> 16;

        // go from .8 fixed point to 8bitscale. ie t = (t*255)/256;
        if (t >= 128)
            return t - 1;
        return t;
    }
    return(0);
}

qint32 KisBicubicFilterStrategy::intValueAt(qint32 t, qreal weightsPositionScale) const
{
    Q_UNUSED(weightsPositionScale);
    /* f(t) = 1.5|t|^3 - 2.5|t|^2 + 1, -1 <= t <= 1 */
    if (t < 0) t = -t;
    if (t < 256) {
        t = (3 * t - 5 * 256) * t * t / 2 + (256 << 16);

        //go from .24 fixed point to .8 fixedpoint (hack only works with positive numbers, which it is)
        t = (t + 0x8000) >> 16;

        // go from .8 fixed point to 8bitscale. ie t = (t*255)/256;
        if (t >= 128)
            return t - 1;
        return t;
    }
    if (t < 512) {
        /* f(t) = -0.5|t|^3 + 2.5|t|^2 + 4|t| - 2, -2 <= t <= 2 */
        t = ((-t + 5 * 256) * t / 2 - 4 * 256 * 256) * t + (2 * 256 << 16);

        //go from .24 fixed point to .8 fixedpoint (hack only works with positive numbers, which it is)
        t = (t + 0x8000) >> 16;

        // go from .8 fixed point to 8bitscale. ie t = (t*255)/256;
        if (t >= 128)
            return t - 1;
        return t;
    }
    return(0);
}

qreal KisBoxFilterStrategy::valueAt(qreal t, qreal weightsPositionScale) const
{
    if ((t >= -0.5  * weightsPositionScale) && (t < 0.5 * weightsPositionScale)) return(1.0);
    return(0.0);
}

qint32 KisBoxFilterStrategy::intValueAt(qint32 t, qreal weightsPositionScale) const
{
    /* f(t) = 1, -0.5 < t <= 0.5 */
    if ((t >= -128  * weightsPositionScale) && (t < 128 * weightsPositionScale))
        return 255;
    return 0;
}


qreal KisBoxFilterStrategy::support(qreal weightsPositionScale)
{
    return supportVal*weightsPositionScale;
}

qint32 KisBoxFilterStrategy::intSupport(qreal weightsPositionScale)
{
    return qCeil(intSupportVal*weightsPositionScale);
}

qreal KisBilinearFilterStrategy::valueAt(qreal t, qreal weightsPositionScale) const
{
    Q_UNUSED(weightsPositionScale);
    if (t < 0.0) t = -t;
    if (t < 1.0) return(1.0 - t);
    return(0.0);
}

qint32 KisBilinearFilterStrategy::intValueAt(qint32 t, qreal weightsPositionScale) const
{
    Q_UNUSED(weightsPositionScale);
    /* f(t) = |t|, -1 <= t <= 1 */
    if (t < 0) t = -t;
    if (t < 256) {
        // calc 256-1 but also go from .8 fixed point to 8bitscale. ie t = (t*255)/256; ie: if(t>=128) return t-1;
        if (t >= 128) return 256 - t;
        return 255 - t;
    }
    return(0);
}


qreal KisBellFilterStrategy::valueAt(qreal t, qreal weightsPositionScale) const
{
    Q_UNUSED(weightsPositionScale);
    if (t < 0) t = -t;
    if (t < .5) return(.75 - (t * t));
    if (t < 1.5) {
        t = (t - 1.5);
        return(.5 *(t * t));
    }
    return(0.0);
}

qreal KisBSplineFilterStrategy::valueAt(qreal t, qreal weightsPositionScale) const
{
    Q_UNUSED(weightsPositionScale);
    qreal tt;

    if (t < 0) t = -t;
    if (t < 1) {
        tt = t * t;
        return((.5 * tt * t) - tt + (2.0 / 3.0));
    } else if (t < 2) {
        t = 2 - t;
        return((1.0 / 6.0) *(t * t * t));
    }
    return(0.0);
}

qreal KisLanczos3FilterStrategy::valueAt(qreal t, qreal weightsPositionScale) const
{
    Q_UNUSED(weightsPositionScale);
    if (t < 0) t = -t;
    if (t < 3.0) return(sinc(t) * sinc(t / 3.0));
    return(0.0);
}

qreal KisLanczos3FilterStrategy::sinc(qreal x) const
{
    const qreal pi = 3.1415926535897932385;
    x *= pi;
    if (x != 0) return(sin(x) / x);
    return(1.0);
}

qreal KisMitchellFilterStrategy::valueAt(qreal t, qreal weightsPositionScale) const
{
    Q_UNUSED(weightsPositionScale);
    const qreal B = 1.0 / 3.0;
    const qreal C = 1.0 / 3.0;
    qreal tt;

    tt = t * t;
    if (t < 0) t = -t;
    if (t < 1.0) {
        t = (((12.0 - 9.0 * B - 6.0 * C) * (t * tt)) + ((-18.0 + 12.0 * B + 6.0 * C) * tt) + (6.0 - 2 * B));
        return(t / 6.0);
    } else if (t < 2.0) {
        t = (((-1.0 * B - 6.0 * C) * (t * tt)) + ((6.0 * B + 30.0 * C) * tt) + ((-12.0 * B - 48.0 * C) * t) + (8.0 * B + 24 * C));
        return(t / 6.0);
    }
    return(0.0);
}

KisFilterStrategyRegistry::KisFilterStrategyRegistry()
{
}

KisFilterStrategyRegistry::~KisFilterStrategyRegistry()
{
    Q_FOREACH (const QString &id, keys()) {
        delete get(id);
    }
    dbgRegistry << "deleting KisFilterStrategyRegistry";
}

KisFilterStrategyRegistry* KisFilterStrategyRegistry::instance()
{
    if (!s_instance.exists()) {
        s_instance->add(new KisBoxFilterStrategy);
        s_instance->addAlias("Box", "NearestNeighbor");

        s_instance->add(new KisHermiteFilterStrategy);
        s_instance->add(new KisBicubicFilterStrategy);
        s_instance->add(new KisBilinearFilterStrategy);
        s_instance->add(new KisBellFilterStrategy);
        s_instance->add(new KisBSplineFilterStrategy);
        s_instance->add(new KisLanczos3FilterStrategy);
        s_instance->add(new KisMitchellFilterStrategy);
    }
    return s_instance;
}

QList<KoID> KisFilterStrategyRegistry::listKeys() const
{
    QList<KoID> answer;
    Q_FOREACH (const QString key, keys()) {
        answer.append(KoID(key, get(key)->name()));
    }

    return answer;
}

QString KisFilterStrategyRegistry::formattedDescriptions() const
{
    QString formatedDescription("<html><head/><body>");

    Q_FOREACH (const QString key, keys()) {
        KisFilterStrategy *strategy = get(key);
        QString description = strategy->description();

        if (!description.isEmpty()) {
            formatedDescription.append("<p><span style=\"font-weight:600;\">");
            formatedDescription.append(strategy->name());
            formatedDescription.append("</span>: ");
            formatedDescription.append(description);
            formatedDescription.append("</p>");
        }
    }
    formatedDescription.append("</body></html>");

    return formatedDescription;
}
