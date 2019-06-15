/*
 * This file is part of the KDE project
 *
 * Copyright (c) 2019 Carl Olsson <carl.olsson@gmail.com>
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

#ifndef PALETTIZE_H
#define PALETTIZE_H

#include <kis_filter.h>
#include <kis_config_widget.h>
#include <kis_filter_configuration.h>
#include <boost/geometry.hpp>
#include <boost/geometry/index/rtree.hpp>
#include <boost/geometry/geometries/point.hpp>
#include <boost/geometry/geometries/register/point.hpp>

class KoColorSet;
class KoResourceItemChooser;
class QGroupBox;
class KoPattern;
class QCheckBox;
class QLineEdit;
class QButtonGroup;
class KisDoubleWidget;

class Palettize : public QObject
{
public:
    Palettize(QObject *parent, const QVariantList &);
};

class KisPalettizeWidget : public KisConfigWidget
{
public:
    KisPalettizeWidget(QWidget* parent = 0);
    void setConfiguration(const KisPropertiesConfigurationSP) override;
    KisPropertiesConfigurationSP configuration() const override;
private:
    KoResourceItemChooser* m_paletteWidget;
    QGroupBox* m_ditherGroupBox;
    QButtonGroup* m_ditherModeGroup;
    KoResourceItemChooser* m_ditherPatternWidget;
    QCheckBox* m_ditherPatternUseAlphaCheckBox;
    QLineEdit* m_ditherNoiseSeedWidget;
    KisDoubleWidget* m_ditherWeightWidget;
};

class KisFilterPalettize : public KisFilter
{
public:
    enum DitherMode {
        Pattern,
        Noise
    };
    KisFilterPalettize();
    static inline KoID id() { return KoID("palettize", i18n("Palettize")); }
    KisConfigWidget* createConfigurationWidget(QWidget* parent, const KisPaintDeviceSP dev, bool useForMasks) const override;
    KisFilterConfigurationSP factoryConfiguration() const override;
    void processImpl(KisPaintDeviceSP device, const QRect &applyRect, const KisFilterConfigurationSP config, KoUpdater *progressUpdater) const override;
};

#endif
