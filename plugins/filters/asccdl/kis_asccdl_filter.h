/*
 * Copyright (c) 2017 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
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
    KisFilterConfigurationSP defaultConfiguration() const override;
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
