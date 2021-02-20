/*
 * This file is part of the KDE project
 *
 * SPDX-FileCopyrightText: 2019 Carl Olsson <carl.olsson@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef PALETTIZE_H
#define PALETTIZE_H

#include "ui_palettize.h"

#include <kis_filter.h>
#include <kis_config_widget.h>
#include <kis_filter_configuration.h>
#include <boost/geometry.hpp>
#include <boost/geometry/index/rtree.hpp>
#include <boost/geometry/geometries/point.hpp>
#include <boost/geometry/geometries/register/point.hpp>

class KisResourceItemChooser;

class Palettize : public QObject
{
public:
    Palettize(QObject *parent, const QVariantList &);
};

class KisPalettizeWidget : public KisConfigWidget, public Ui::Palettize
{
public:
    KisPalettizeWidget(QWidget* parent = 0);
    void setConfiguration(const KisPropertiesConfigurationSP) override;
    KisPropertiesConfigurationSP configuration() const override;
private:
    KisResourceItemChooser* m_paletteWidget;
    KisResourceItemChooser* m_ditherPatternWidget;
};

class KisFilterPalettize : public KisFilter
{
public:
    enum Colorspace {
        Lab,
        RGB
    };
    enum AlphaMode {
        Clip,
        Index,
        Dither
    };
    enum ThresholdMode {
        Pattern,
        Noise
    };
    enum PatternValueMode {
        Auto,
        Lightness,
        Alpha
    };
    enum ColorMode {
        PerChannelOffset,
        NearestColors
    };
    KisFilterPalettize();
    static inline KoID id() { return KoID("palettize", i18n("Palettize")); }
    KisConfigWidget* createConfigurationWidget(QWidget* parent, const KisPaintDeviceSP dev, bool useForMasks) const override;
    KisFilterConfigurationSP factoryConfiguration(KisResourcesInterfaceSP resourcesInterface) const override;
    KisFilterConfigurationSP defaultConfiguration(KisResourcesInterfaceSP resourcesInterface) const override;
    void processImpl(KisPaintDeviceSP device, const QRect &applyRect, const KisFilterConfigurationSP config, KoUpdater *progressUpdater) const override;
};

#endif
