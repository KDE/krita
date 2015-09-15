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

#ifndef KIS_FILTER_STRATEGY_H_
#define KIS_FILTER_STRATEGY_H_

#include <klocalizedstring.h>

#include "kis_types.h"
#include "KoGenericRegistry.h"
#include "KoID.h"
#include "kritaimage_export.h"

class KRITAIMAGE_EXPORT KisFilterStrategy
{
public:
    KisFilterStrategy(KoID id) : m_id(id) {}
    virtual ~KisFilterStrategy() {}

    QString id() {
        return m_id.id();
    }
    QString name() {
        return m_id.name();
    }
    virtual qreal valueAt(qreal /*t*/) const {
        return 0;
    }
    virtual qint32 intValueAt(qint32 t) const {
        return qint32(255*valueAt(t / 256.0));
    }
    qreal support() {
        return supportVal;
    }
    qint32 intSupport() {
        return intSupportVal;
    }
    virtual bool boxSpecial() {
        return false;
    }
    virtual QString description() {
        return QString("");
    }

protected:
    qreal supportVal;
    qint32 intSupportVal;
    KoID m_id;
};

class KRITAIMAGE_EXPORT KisHermiteFilterStrategy : public KisFilterStrategy
{
public:
    KisHermiteFilterStrategy() : KisFilterStrategy(KoID("Hermite", i18n("Hermite"))) {
        supportVal = 1.0; intSupportVal = 256;
    }
    virtual ~KisHermiteFilterStrategy() {}

    virtual qint32 intValueAt(qint32 t) const;
    virtual qreal valueAt(qreal t) const;
};

class KRITAIMAGE_EXPORT KisBicubicFilterStrategy : public KisFilterStrategy
{
public:
    KisBicubicFilterStrategy() : KisFilterStrategy(KoID("Bicubic", i18n("Bicubic"))) {
        supportVal = 2.0; intSupportVal = 512;
    }
    virtual ~KisBicubicFilterStrategy() {}

    virtual QString description() {
        return i18n("Adds pixels using the color of surrounding pixels. Produces smoother tonal gradations than Bilinear.");
    }

    virtual qint32 intValueAt(qint32 t) const;
};
class KRITAIMAGE_EXPORT KisBoxFilterStrategy : public KisFilterStrategy
{
public:
    KisBoxFilterStrategy() : KisFilterStrategy(KoID("Box", i18n("Box"))) {
        supportVal = 0.5; intSupportVal = 128;
    }
    virtual ~KisBoxFilterStrategy() {}

    virtual QString description() {
        return i18n("Replicate pixels in the image. Preserves all the original detail, but can produce jagged effects.");
    }

    virtual qint32 intValueAt(qint32 t) const;
    virtual qreal valueAt(qreal t) const;
    virtual bool boxSpecial() {
        return true;
    }
};

class KRITAIMAGE_EXPORT KisBilinearFilterStrategy : public KisFilterStrategy
{
public:
    KisBilinearFilterStrategy() : KisFilterStrategy(KoID("Bilinear", i18n("Bilinear"))) {
        supportVal = 1.0; intSupportVal = 256;
    }
    virtual ~KisBilinearFilterStrategy() {}

    virtual QString description() {
        return i18n("Adds pixels averaging the color values of surrounding pixels. Produces medium quality results when the image is scaled from half to two times the original size.");
    }

    virtual qint32 intValueAt(qint32 t) const;
    virtual qreal valueAt(qreal t) const;
};

class KRITAIMAGE_EXPORT KisBellFilterStrategy : public KisFilterStrategy
{
public:
    KisBellFilterStrategy() : KisFilterStrategy(KoID("Bell", i18n("Bell"))) {
        supportVal = 1.5; intSupportVal = 128 + 256;
    }
    virtual ~KisBellFilterStrategy() {}

    virtual qreal valueAt(qreal t) const;
};

class KRITAIMAGE_EXPORT KisBSplineFilterStrategy : public KisFilterStrategy
{
public:
    KisBSplineFilterStrategy() : KisFilterStrategy(KoID("BSpline", i18n("BSpline"))) {
        supportVal = 2.0; intSupportVal = 512;
    }
    virtual ~KisBSplineFilterStrategy() {}

    virtual qreal valueAt(qreal t) const;
};

class KRITAIMAGE_EXPORT KisLanczos3FilterStrategy : public KisFilterStrategy
{
public:
    KisLanczos3FilterStrategy() : KisFilterStrategy(KoID("Lanczos3", i18n("Lanczos3"))) {
        supportVal = 3.0; intSupportVal = 768;
    }
    virtual ~KisLanczos3FilterStrategy() {}

    virtual QString description() {
        return i18n("Offers similar results than Bicubic, but maybe a little bit sharper. Can produce light and dark halos along strong edges.");
    }

    virtual qreal valueAt(qreal t) const;
private:
    qreal sinc(qreal x) const;
};

class KRITAIMAGE_EXPORT  KisMitchellFilterStrategy : public KisFilterStrategy
{
public:
    KisMitchellFilterStrategy() : KisFilterStrategy(KoID("Mitchell", i18n("Mitchell"))) {
        supportVal = 2.0; intSupportVal = 256;
    }
    virtual ~KisMitchellFilterStrategy() {}

    virtual qreal valueAt(qreal t) const;
};

class KRITAIMAGE_EXPORT KisFilterStrategyRegistry : public KoGenericRegistry<KisFilterStrategy *>
{

public:

    KisFilterStrategyRegistry();
    ~KisFilterStrategyRegistry();
    static KisFilterStrategyRegistry* instance();

    /**
     * This function return a list of all the keys in KoID format by using the name() method
     * on the objects stored in the registry.
     */
    QList<KoID> listKeys() const;

    /**
     * This function return a string formated in HTML that contains the descriptions of all objects
     * (with a non empty description) stored in the registry.
     */
    QString formatedDescriptions() const;

private:

    KisFilterStrategyRegistry(const KisFilterStrategyRegistry&);
    KisFilterStrategyRegistry operator=(const KisFilterStrategyRegistry&);

};

#endif // KIS_FILTER_STRATEGY_H_
