/*
 * KDE. Krita Project.
 *
 * SPDX-FileCopyrightText: 2024 Deif Lou <ginoba@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_PROPAGATE_COLORS_FILTER_H
#define KIS_PROPAGATE_COLORS_FILTER_H

#include <QObject>
#include <QVector>

#include <filter/kis_filter.h>
#include <kis_filter_configuration.h>

class KisConfigWidget;

class KisPropagateColorsFilter : public KisFilter
{
public:
    KisPropagateColorsFilter();

    void processImpl(KisPaintDeviceSP device,
                     const QRect& applyRect,
                     const KisFilterConfigurationSP config,
                     KoUpdater *progressUpdater) const override;

    KisFilterConfigurationSP factoryConfiguration(KisResourcesInterfaceSP resourcesInterface) const override;
    KisConfigWidget* createConfigurationWidget(QWidget *parent, const KisPaintDeviceSP dev, bool useForMasks) const override;
    bool needsTransparentPixels(const KisFilterConfigurationSP config, const KoColorSpace *cs) const override;
    QRect neededRect(const QRect &rect, const KisFilterConfigurationSP config, int lod) const override;
    QRect changedRect(const QRect &rect, const KisFilterConfigurationSP config, int lod) const override;
};

#endif
