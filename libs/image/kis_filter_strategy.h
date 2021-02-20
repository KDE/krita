/*
 *  SPDX-FileCopyrightText: 2004 Michael Thaler <michael.thaler@physik.tu-muenchen.de>
 *  SPDX-FileCopyrightText: 2005 C. Boemann <cbo@boemann.dk>
 *  SPDX-FileCopyrightText: 2013 Juan Palacios <jpalaciosdev@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_FILTER_STRATEGY_H_
#define KIS_FILTER_STRATEGY_H_

#include <klocalizedstring.h>

#include "KoGenericRegistry.h"
#include "KoID.h"
#include "kritaimage_export.h"
#include <QSize>

class KRITAIMAGE_EXPORT KisFilterStrategy
{
public:
    KisFilterStrategy(KoID id) : m_id(id) {}
    virtual ~KisFilterStrategy() {  }

    QString id() {
        return m_id.id();
    }
    QString name() {
        return m_id.name();
    }
    virtual qreal valueAt(qreal t, qreal weightsPositionScale) const {
        Q_UNUSED(t);
        Q_UNUSED(weightsPositionScale);
        return 0;
    }
    virtual qint32 intValueAt(qint32 t, qreal weightsPositionScale) const {
        return qint32(255*valueAt(t / 256.0, weightsPositionScale));
    }
    virtual qreal support(qreal weightsPositionScale) {
        Q_UNUSED(weightsPositionScale);
        return supportVal;
    }
    virtual qint32 intSupport(qreal weightsPositionScale) {
        Q_UNUSED(weightsPositionScale);
        return intSupportVal;
    }
    virtual QString description() {
        return QString();
    }

protected:
    qreal supportVal {0.0};
    qint32 intSupportVal {0};
    KoID m_id;
};

class KRITAIMAGE_EXPORT KisHermiteFilterStrategy : public KisFilterStrategy
{
public:
    KisHermiteFilterStrategy() : KisFilterStrategy(KoID("Hermite", i18n("Hermite"))) {
        supportVal = 1.0; intSupportVal = 256;
    }
    ~KisHermiteFilterStrategy() override {}

    qint32 intValueAt(qint32 t, qreal weightsPositionScale) const override;
    qreal valueAt(qreal t, qreal weightsPositionScale) const override;
};

class KRITAIMAGE_EXPORT KisBicubicFilterStrategy : public KisFilterStrategy
{
public:
    KisBicubicFilterStrategy() : KisFilterStrategy(KoID("Bicubic", i18n("Bicubic"))) {
        supportVal = 2.0; intSupportVal = 512;
    }
    ~KisBicubicFilterStrategy() override {}

    QString description() override {
        return i18n("Adds pixels using the color of surrounding pixels. Produces smoother tonal gradations than Bilinear.");
    }

    qint32 intValueAt(qint32 t, qreal weightsPositionScale) const override;
};
class KRITAIMAGE_EXPORT KisBoxFilterStrategy : public KisFilterStrategy
{
public:
    KisBoxFilterStrategy() : KisFilterStrategy(KoID("NearestNeighbor", i18n("Nearest Neighbor"))) {
        // 0.5 and 128, but with a bit of margin to ensure the correct pixel will be used
        // even in case of calculation errors
        supportVal = 0.51; intSupportVal = 129;
    }
    ~KisBoxFilterStrategy() override {}

    QString description() override {
        return i18n("Replicate pixels in the image. Preserves all the original detail, but can produce jagged effects.");
    }

    virtual qreal support(qreal weightsPositionScale) override;
    virtual qint32 intSupport(qreal weightsPositionScale) override;


    qint32 intValueAt(qint32 t, qreal weightsPositionScale) const override;
    qreal valueAt(qreal t, qreal weightsPositionScale) const override;
};

class KRITAIMAGE_EXPORT KisBilinearFilterStrategy : public KisFilterStrategy
{
public:
    KisBilinearFilterStrategy() : KisFilterStrategy(KoID("Bilinear", i18n("Bilinear"))) {
        supportVal = 1.0; intSupportVal = 256;
    }
    ~KisBilinearFilterStrategy() override {}

    QString description() override {
        return i18n("Adds pixels averaging the color values of surrounding pixels. Produces medium quality results when the image is scaled from half to two times the original size.");
    }

    qint32 intValueAt(qint32 t, qreal weightsPositionScale) const override;
    qreal valueAt(qreal t, qreal weightsPositionScale) const override;
};

class KRITAIMAGE_EXPORT KisBellFilterStrategy : public KisFilterStrategy
{
public:
    KisBellFilterStrategy() : KisFilterStrategy(KoID("Bell", i18n("Bell"))) {
        supportVal = 1.5; intSupportVal = 128 + 256;
    }
    ~KisBellFilterStrategy() override {}

    qreal valueAt(qreal t, qreal weightsPositionScale) const override;
};

class KRITAIMAGE_EXPORT KisBSplineFilterStrategy : public KisFilterStrategy
{
public:
    KisBSplineFilterStrategy() : KisFilterStrategy(KoID("BSpline", i18n("BSpline"))) {
        supportVal = 2.0; intSupportVal = 512;
    }
    ~KisBSplineFilterStrategy() override {}

    qreal valueAt(qreal t, qreal weightsPositionScale) const override;
};

class KRITAIMAGE_EXPORT KisLanczos3FilterStrategy : public KisFilterStrategy
{
public:
    KisLanczos3FilterStrategy() : KisFilterStrategy(KoID("Lanczos3", i18n("Lanczos3"))) {
        supportVal = 3.0; intSupportVal = 768;
    }
    ~KisLanczos3FilterStrategy() override {}

    QString description() override {
        return i18n("Offers similar results than Bicubic, but maybe a little bit sharper. Can produce light and dark halos along strong edges.");
    }

    qreal valueAt(qreal t, qreal weightsPositionScale) const override;
private:
    qreal sinc(qreal x) const;
};

class KRITAIMAGE_EXPORT  KisMitchellFilterStrategy : public KisFilterStrategy
{
public:
    KisMitchellFilterStrategy() : KisFilterStrategy(KoID("Mitchell", i18n("Mitchell"))) {
        supportVal = 2.0; intSupportVal = 256;
    }
    ~KisMitchellFilterStrategy() override {}

    qreal valueAt(qreal t, qreal weightsPositionScale) const override;
};

class KRITAIMAGE_EXPORT KisFilterStrategyRegistry : public KoGenericRegistry<KisFilterStrategy *>
{

public:

    KisFilterStrategyRegistry();
    ~KisFilterStrategyRegistry() override;
    static KisFilterStrategyRegistry* instance();

    /**
     * This function return a list of all the keys in KoID format by using the name() method
     * on the objects stored in the registry.
     */
    QList<KoID> listKeys() const;

    /**
     * This function return a string formatted in HTML that contains the descriptions of all objects
     * (with a non empty description) stored in the registry.
     */
    QString formattedDescriptions() const;

    /**
     * Try to select an appropriate image filtering strategy based on original and desired parameters.
     */
    KisFilterStrategy* autoFilterStrategy(QSize originalSize, QSize desiredSize) const;

private:

    KisFilterStrategyRegistry(const KisFilterStrategyRegistry&);
    KisFilterStrategyRegistry operator=(const KisFilterStrategyRegistry&);

};

#endif // KIS_FILTER_STRATEGY_H_
