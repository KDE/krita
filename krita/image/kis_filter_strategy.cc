/*
 *  Copyright (c) 2004 Michael Thaler <michael.thaler@physik.tu-muenchen.de>
 *  Copyright (c) 2005 Casper Boemann <cbr@boemann.dk>
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
#include <kdebug.h>
#include <klocale.h>
#include "kis_debug_areas.h"
#include "kis_filter_strategy.h"
#include <math.h>

double KisHermiteFilterStrategy::valueAt(double t) const {
        /* f(t) = 2|t|^3 - 3|t|^2 + 1, -1 <= t <= 1 */
        if(t < 0.0) t = -t;
        if(t < 1.0) return((2.0 * t - 3.0) * t * t + 1.0);
        return(0.0);
}

quint32 KisHermiteFilterStrategy::intValueAt(qint32 t) const {
        /* f(t) = 2|t|^3 - 3|t|^2 + 1, -1 <= t <= 1 */
        if(t < 0) t = -t;
        if(t < 256)
    {
        t =(2 * t - 3*256) * t * t +(256<<16);

        //go from .24 fixed point to .8 fixedpoint (hack only works with positve numbers, which it is)
        t = (t + 0x8000) >> 16;

        // go from .8 fixed point to 8bitscale. ie t = (t*255)/256;
        if(t >= 128)
            return t - 1;
        return t;
    }
        return(0);
}

double KisBoxFilterStrategy::valueAt(double t) const {
        if((t > -0.5) && (t <= 0.5)) return(1.0);
        return(0.0);
}

quint32 KisBoxFilterStrategy::intValueAt(qint32 t) const {
        /* f(t) = 1, -0.5 < t <= 0.5 */
    if((t > -128) && (t <= 128))
        return 255;
    return 0;
}

double KisTriangleFilterStrategy::valueAt(double t) const {
        if(t < 0.0) t = -t;
        if(t < 1.0) return(1.0 - t);
        return(0.0);
}

quint32 KisTriangleFilterStrategy::intValueAt(qint32 t) const {
        /* f(t) = |t|, -1 <= t <= 1 */
        if(t < 0) t = -t;
        if(t < 256)
    {
         // calc 256-1 but also go from .8 fixed point to 8bitscale. ie t = (t*255)/256; ie: if(t>=128) return t-1;
        if(t>=128) return 256 - t;
        return 255 - t;
    }
        return(0);
}


double KisBellFilterStrategy::valueAt(double t) const {
        if(t < 0) t = -t;
        if(t < .5) return(.75 - (t * t));
        if(t < 1.5) {
                t = (t - 1.5);
                return(.5 * (t * t));
        }
        return(0.0);
}

double KisBSplineFilterStrategy::valueAt(double t) const {
        double tt;

        if(t < 0) t = -t;
        if(t < 1) {
                tt = t * t;
                return((.5 * tt * t) - tt + (2.0 / 3.0));
        } else if(t < 2) {
                t = 2 - t;
                return((1.0 / 6.0) * (t * t * t));
        }
        return(0.0);
}

double KisLanczos3FilterStrategy::valueAt(double t) const {
        if(t < 0) t = -t;
        if(t < 3.0) return(sinc(t) * sinc(t/3.0));
        return(0.0);
}

double KisLanczos3FilterStrategy::sinc(double x) const {
        const double pi=3.1415926535897932385;
        x *= pi;
        if(x != 0) return(sin(x) / x);
        return(1.0);
}

double KisMitchellFilterStrategy::valueAt(double t) const {
        const double B=1.0/3.0;
        const double C=1.0/3.0;
        double tt;

        tt = t * t;
        if(t < 0) t = -t;
        if(t < 1.0) {
                t = (((12.0 - 9.0 * B - 6.0 * C) * (t * tt)) + ((-18.0 + 12.0 * B + 6.0 * C) * tt) + (6.0 - 2 * B));
                return(t / 6.0);
        } else if(t < 2.0) {
                t = (((-1.0 * B - 6.0 * C) * (t * tt)) + ((6.0 * B + 30.0 * C) * tt) + ((-12.0 * B - 48.0 * C) * t) + (8.0 * B + 24 * C));
                return(t / 6.0);
                }
        return(0.0);
}

KisFilterStrategyRegistry *KisFilterStrategyRegistry::m_singleton = 0;

KisFilterStrategyRegistry::KisFilterStrategyRegistry()
{
    Q_ASSERT(KisFilterStrategyRegistry::m_singleton == 0);
    KisFilterStrategyRegistry::m_singleton = this;
}

KisFilterStrategyRegistry::~KisFilterStrategyRegistry()
{
}

KisFilterStrategyRegistry* KisFilterStrategyRegistry::instance()
{
    if(KisFilterStrategyRegistry::m_singleton == 0)
    {
        KisFilterStrategyRegistry::m_singleton = new KisFilterStrategyRegistry();
        Q_CHECK_PTR(KisFilterStrategyRegistry::m_singleton);
        m_singleton->add(new KisHermiteFilterStrategy);
        m_singleton->add(new KisBoxFilterStrategy);
        m_singleton->add(new KisTriangleFilterStrategy);
        m_singleton->add(new KisBellFilterStrategy);
        m_singleton->add(new KisBSplineFilterStrategy);
        m_singleton->add(new KisLanczos3FilterStrategy);
        m_singleton->add(new KisMitchellFilterStrategy);
    }
    return KisFilterStrategyRegistry::m_singleton;
}

