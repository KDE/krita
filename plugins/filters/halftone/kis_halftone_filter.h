/*
 * This file is part of Krita
 *
 * Copyright (c) 2016 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
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

#ifndef KISHALFTONEFILTER_H
#define KISHALFTONEFILTER_H

#include <QObject>
#include <filter/kis_filter.h>
#include <kis_filter_configuration.h>
#include <kis_config_widget.h>

#include "ui_wdg_halftone_filter.h"

class WdgHalftone;



class KritaHalftone : public QObject
{
    Q_OBJECT
public:
    KritaHalftone(QObject *parent, const QVariantList &);
    ~KritaHalftone() override;
};

/**
 * @brief The kisHalftoneFilter class
 * This filter will allow the user to input an image and have it be approximated with
 * a halftone pattern. https://en.wikipedia.org/wiki/Halftone
 *
 * The primary usecase of such a filter is for specialized printing techniques, but for
 * many people the half-tone pattern also serves as a neutral pattern that is more pleasant
 * than plain flat look. The half tone in this case also becomes a stylistic technique.
 *
 * Based on that, there's a few ways a user could want to use this techique:
 * 1. Per-component. Per patch, each component will have a halftone approximated.
 * 2. Intensity only. The relative luminosity of the patch is determined and will be used
 * for the approximation, resulting in a black/white pattern.
 * 3. Intensity and then two colors mapped to the black/white pattern.
 *
 * On top of that, the pattern can be rotated, the shape can be chosen, and the user will want to
 * decide whether or not to use antialiasing(as printers themselves give
 * inefficient results with anti-aliasing).
 *
 * As of currently, 2 and 3 can be done. 1 is not impossible to code with some creative usage of composite
 * modes, but might be a little slow.
 */

class KisHalftoneFilter : public KisFilter
{
public:
    KisHalftoneFilter();

    static inline KoID id() {
        return KoID("halftone", i18n("Halftone"));
    }

    void processImpl(KisPaintDeviceSP device,
                     const QRect& applyRect,
                     const KisFilterConfigurationSP config,
                     KoUpdater *progressUpdater) const override;

    KisFilterConfigurationSP factoryConfiguration() const override;

    KisConfigWidget *createConfigurationWidget(QWidget *parent, const KisPaintDeviceSP dev) const override;

private:
    QPolygonF m_gridPoints;
};

class KisHalftoneConfigWidget : public KisConfigWidget
{
    Q_OBJECT
public:
    KisHalftoneConfigWidget(QWidget *parent, KisPaintDeviceSP dev);
    ~KisHalftoneConfigWidget() override;

    KisPropertiesConfigurationSP configuration() const override;
    void setConfiguration(const KisPropertiesConfigurationSP config) override;
    Ui::WdgHalftone m_page;

};

#endif // KISHALFTONEFILTER_H
