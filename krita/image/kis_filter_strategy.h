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

#ifndef KIS_FILTER_STRATEGY_H_
#define KIS_FILTER_STRATEGY_H_

#include <klocale.h>

#include "kis_types.h"
#include "KoGenericRegistry.h"
#include "KoID.h"
#include "krita_export.h"

class KRITAIMAGE_EXPORT KisFilterStrategy
{
    public:
        KisFilterStrategy(KoID id) : m_id(id) {}
        virtual ~KisFilterStrategy() {}

        KoID id() {return m_id;};
        virtual double valueAt(double /*t*/) const {return 0;};
        virtual quint32 intValueAt(qint32 t) const {return quint32(255*valueAt(t/256.0));};
        double support() { return supportVal;};
        quint32 intSupport() { return intSupportVal;};
        virtual bool boxSpecial() { return false;};
    protected:
        double supportVal;
        quint32 intSupportVal;
        KoID m_id;
};

class KRITAIMAGE_EXPORT KisHermiteFilterStrategy : public KisFilterStrategy
{
    public:
        KisHermiteFilterStrategy() : KisFilterStrategy(KoID("Hermite", i18n("Hermite")))
            {supportVal = 1.0; intSupportVal = 256;}
        virtual ~KisHermiteFilterStrategy() {}

        virtual quint32 intValueAt(qint32 t) const;
        virtual double valueAt(double t) const;
};

class KRITAIMAGE_EXPORT KisBoxFilterStrategy : public KisFilterStrategy
{
    public:
        KisBoxFilterStrategy() : KisFilterStrategy(KoID("Box", i18n("Box")))
             {supportVal = 0.5; intSupportVal = 128;}
        virtual ~KisBoxFilterStrategy() {}

        virtual quint32 intValueAt(qint32 t) const;
        virtual double valueAt(double t) const;
        virtual bool boxSpecial() { return true;};
};

class KRITAIMAGE_EXPORT KisTriangleFilterStrategy : public KisFilterStrategy
{
    public:
        KisTriangleFilterStrategy() : KisFilterStrategy(KoID("Triangle", i18n("Triangle aka (bi)linear")))
            {supportVal = 1.0; intSupportVal = 256;}
        virtual ~KisTriangleFilterStrategy() {}

        virtual quint32 intValueAt(qint32 t) const;
        virtual double valueAt(double t) const;
};

class KRITAIMAGE_EXPORT KisBellFilterStrategy : public KisFilterStrategy
{
    public:
        KisBellFilterStrategy() : KisFilterStrategy(KoID("Bell", i18n("Bell")))
            {supportVal = 1.5; intSupportVal = 128+256;}
        virtual ~KisBellFilterStrategy() {}

        virtual double valueAt(double t) const;
};

class KRITAIMAGE_EXPORT KisBSplineFilterStrategy : public KisFilterStrategy
{
    public:
        KisBSplineFilterStrategy() : KisFilterStrategy(KoID("BSpline", i18n("BSpline")))
            {supportVal = 2.0; intSupportVal = 512;}
        virtual ~KisBSplineFilterStrategy() {}

        virtual double valueAt(double t) const;
};

class KRITAIMAGE_EXPORT KisLanczos3FilterStrategy : public KisFilterStrategy
{
    public:
        KisLanczos3FilterStrategy() : KisFilterStrategy(KoID("Lanczos3", i18n("Lanczos3")))
            {supportVal = 3.0; intSupportVal = 768;}
        virtual ~KisLanczos3FilterStrategy() {}

        virtual double valueAt(double t) const;
    private:
        double sinc(double x) const;
};

class KRITAIMAGE_EXPORT  KisMitchellFilterStrategy : public KisFilterStrategy
{
    public:
        KisMitchellFilterStrategy() : KisFilterStrategy(KoID("Mitchell", i18n("Mitchell")))
            {supportVal = 2.0; intSupportVal = 256;}
        virtual ~KisMitchellFilterStrategy() {}

        virtual double valueAt(double t) const;
};

class KRITAIMAGE_EXPORT KisFilterStrategyRegistry : public KoGenericRegistry<KisFilterStrategy *>
{
public:
    virtual ~KisFilterStrategyRegistry();

    static KisFilterStrategyRegistry* instance();

private:
    KisFilterStrategyRegistry();
     KisFilterStrategyRegistry(const KisFilterStrategyRegistry&);
     KisFilterStrategyRegistry operator=(const KisFilterStrategyRegistry&);

private:
    static KisFilterStrategyRegistry *m_singleton;
};

#endif // KIS_FILTER_STRATEGY_H_
