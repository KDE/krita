/*
 * SPDX-FileCopyrightText: 2017 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_ASCCDL_FILTER_H
#define KIS_ASCCDL_FILTER_H

#include <filter/kis_filter.h>
#include "filter/kis_color_transformation_filter.h"

class KritaASCCDL : public QObject
{
    Q_OBJECT
public:
    KritaASCCDL(QObject *parent, const QVariantList &);
    ~KritaASCCDL() override;
};

class KisFilterASCCDL: public KisColorTransformationFilter
{
public:
    KisFilterASCCDL();
public:

    static inline KoID id() {
        return KoID("asc-cdl", i18n("Slope, Offset, Power(ASC-CDL)"));
    }
    KoColorTransformation *createTransformation(const KoColorSpace* cs, const KisFilterConfigurationSP config) const override;
    KisConfigWidget *createConfigurationWidget(QWidget* parent, const KisPaintDeviceSP dev, bool useForMasks) const override;
    bool needsTransparentPixels(const KisFilterConfigurationSP config, const KoColorSpace *cs) const override;
protected:
    KisFilterConfigurationSP defaultConfiguration(KisResourcesInterfaceSP resourcesInterface) const override;
};

class KisASCCDLTransformation : public KoColorTransformation
{
public:
    KisASCCDLTransformation(const KoColorSpace *cs, KoColor slope, KoColor offset, KoColor power);
    void transform(const quint8* src, quint8* dst, qint32 nPixels) const override;
private:
    QVector<float> m_slope;
    QVector<float> m_offset;
    QVector<float> m_power;
    const KoColorSpace *m_cs;
};

#endif // KIS_ASCCDL_H
