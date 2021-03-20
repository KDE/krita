/*
 * SPDX-FileCopyrightText: 2014 Manuel Riecke <spell1337@gmail.com>
 *
 * SPDX-License-Identifier: ICS
 */

#ifndef INDEXCOLORS_H
#define INDEXCOLORS_H

#include <QObject>
#include <QVariant>
#include "filter/kis_color_transformation_filter.h"
#include "kis_config_widget.h"
#include <KoColor.h>

#include "indexcolorpalette.h"

class IndexColors : public QObject
{
    Q_OBJECT
public:
    IndexColors(QObject *parent, const QVariantList &);
    ~IndexColors() override;
};

class KisFilterIndexColors : public KisColorTransformationFilter
{
public:
    KisFilterIndexColors();
public:
    KoColorTransformation* createTransformation(const KoColorSpace* cs, const KisFilterConfigurationSP config) const override;
    KisConfigWidget* createConfigurationWidget(QWidget* parent, const KisPaintDeviceSP dev, bool useForMasks) const override;
    static inline KoID id() {
        return KoID("indexcolors", i18n("Index Colors"));
    }
protected:
    KisFilterConfigurationSP defaultConfiguration(KisResourcesInterfaceSP resourcesInterface) const override;
};

class KisIndexColorTransformation : public KoColorTransformation
{
public:
    KisIndexColorTransformation(IndexColorPalette palette, const KoColorSpace* cs, int alphaSteps);
    void transform(const quint8* src, quint8* dst, qint32 nPixels) const override;
private:
    const KoColorSpace* m_colorSpace;
    quint32 m_psize;
    IndexColorPalette m_palette;
    quint16 m_alphaStep;
    quint16 m_alphaHalfStep;
};

#endif
