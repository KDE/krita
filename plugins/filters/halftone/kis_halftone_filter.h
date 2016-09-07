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


class KritaHalftone : public QObject
{
    Q_OBJECT
public:
    KritaHalftone(QObject *parent, const QVariantList &);
    virtual ~KritaHalftone();
};

/**
 * @brief The kisHalftoneFilter class
 * This filter will allow the user to input an image and have it be approximated with
 * a halftone pattern. https://en.wikipedia.org/wiki/Halftone
 *
 * The primary usecase of such a filter is for specialised printing techniques, but for
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
 */

class KisHalftoneFilter : public KisFilter
{
public:
    KisHalftoneFilter();

    static inline KoID id() {
        return KoID("halftone", i18n("Halftone"));
    }

    virtual void processImpl(KisPaintDeviceSP device,
                     const QRect& applyRect,
                     const KisFilterConfiguration* config,
                     KoUpdater *progressUpdater) const;

    virtual KisFilterConfiguration *factoryConfiguration(const KisPaintDeviceSP) const;

    //virtual KisConfigWidget *createConfigurationWidget(QWidget *parent, const KisPaintDeviceSP dev) const;

private:
    QPolygonF m_gridPoints;
};

#endif // KISHALFTONEFILTER_H
