/*
 *  SPDX-FileCopyrightText: 2014 Manuel Riecke <spell1337@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef POSTERIZE_H
#define POSTERIZE_H

#include <QObject>
#include <QVariant>
#include "filter/kis_color_transformation_filter.h"
#include "kis_config_widget.h"

class Posterize : public QObject
{
    Q_OBJECT
public:
    Posterize(QObject *parent, const QVariantList &);
    ~Posterize() override;
};

class KisFilterPosterize : public KisColorTransformationFilter
{
public:
    KisFilterPosterize();
public:
    KoColorTransformation* createTransformation(const KoColorSpace* cs, const KisFilterConfigurationSP config) const override;
    KisConfigWidget* createConfigurationWidget(QWidget* parent, const KisPaintDeviceSP dev, bool useForMasks) const override;
    static inline KoID id() {
        return KoID("posterize", i18n("Posterize"));
    }
protected:
    KisFilterConfigurationSP defaultConfiguration(KisResourcesInterfaceSP resourcesInterface) const override;
};

class KisPosterizeColorTransformation : public KoColorTransformation
{
public:
    KisPosterizeColorTransformation(int steps, const KoColorSpace* cs);
    ~KisPosterizeColorTransformation() override;
    void transform(const quint8* src, quint8* dst, qint32 nPixels) const override;
private:
    const KoColorSpace* m_colorSpace;
    quint32 m_psize;
    quint16 m_step;
    quint16 m_halfStep;
    KoColorConversionTransformation* m_fromConversion;
    KoColorConversionTransformation* m_toConversion;
};

#endif
