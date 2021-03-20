/*
 * SPDX-FileCopyrightText: 2017 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KIS_EDGE_DETECTION_FILTER_H
#define KIS_EDGE_DETECTION_FILTER_H

#include "filter/kis_filter.h"

#include <Eigen/Core>

class KritaEdgeDetectionFilter : public QObject
{
    Q_OBJECT
public:
    KritaEdgeDetectionFilter(QObject *parent, const QVariantList &);
    ~KritaEdgeDetectionFilter() override;
};

class KisEdgeDetectionFilter : public KisFilter
{
public:
    KisEdgeDetectionFilter();
    void processImpl(KisPaintDeviceSP device,
                     const QRect& rect,
                     const KisFilterConfigurationSP config,
                     KoUpdater* progressUpdater
                     ) const override;
    static inline KoID id() {
        return KoID("edge detection", i18n("Edge Detection"));
    }

    KisFilterConfigurationSP defaultConfiguration(KisResourcesInterfaceSP resourcesInterface) const override;
public:
    KisConfigWidget * createConfigurationWidget(QWidget* parent, const KisPaintDeviceSP dev, bool useForMasks) const override;
    QRect neededRect(const QRect & rect, const KisFilterConfigurationSP _config, int lod) const override;
    QRect changedRect(const QRect & rect, const KisFilterConfigurationSP _config, int lod) const override;
};

#endif // KIS_EDGE_DETECTION_FILTER_H
