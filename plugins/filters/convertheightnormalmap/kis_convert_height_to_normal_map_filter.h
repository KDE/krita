/*
 * SPDX-FileCopyrightText: 2017 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KIS_CONVERT_HEIGHT_TO_NORMAL_MAP_FILTER_H
#define KIS_CONVERT_HEIGHT_TO_NORMAL_MAP_FILTER_H

#include "filter/kis_filter.h"

class KritaConvertHeightToNormalMapFilter : public QObject
{
    Q_OBJECT
public:
    KritaConvertHeightToNormalMapFilter(QObject *parent, const QVariantList &);
    ~KritaConvertHeightToNormalMapFilter() override;
};

class KisConvertHeightToNormalMapFilter : public KisFilter
{
public:
    KisConvertHeightToNormalMapFilter();
    void processImpl(KisPaintDeviceSP device,
                     const QRect& rect,
                     const KisFilterConfigurationSP config,
                     KoUpdater* progressUpdater
                     ) const override;
    static inline KoID id() {
        return KoID("height to normal", i18n("Height to Normal Map"));
    }

    KisFilterConfigurationSP defaultConfiguration(KisResourcesInterfaceSP resourcesInterface) const override;
public:
    KisConfigWidget * createConfigurationWidget(QWidget* parent, const KisPaintDeviceSP dev, bool useForMasks) const override;
    QRect neededRect(const QRect & rect, const KisFilterConfigurationSP _config, int lod) const override;
    QRect changedRect(const QRect & rect, const KisFilterConfigurationSP _config, int lod) const override;
};


#endif // KIS_CONVERT_HEIGHT_TO_NORMAL_MAP_FILTER_H
