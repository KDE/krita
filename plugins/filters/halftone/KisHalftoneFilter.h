/*
 * KDE. Krita Project.
 *
 * SPDX-FileCopyrightText: 2020 Deif Lou <ginoba@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_HALFTONE_FILTER_H
#define KIS_HALFTONE_FILTER_H

#include <QObject>
#include <QVector>

#include <filter/kis_filter.h>
#include <kis_filter_configuration.h>
#include <kis_cached_paint_device.h>

#include "KisHalftoneFilterConfiguration.h"

class KisConfigWidget;

class KritaHalftone : public QObject
{
    Q_OBJECT
public:
    KritaHalftone(QObject *parent, const QVariantList &);
    ~KritaHalftone() override;
};

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

    KisFilterConfigurationSP defaultConfiguration(KisResourcesInterfaceSP resourcesInterface) const override;
    KisFilterConfigurationSP factoryConfiguration(KisResourcesInterfaceSP resourcesInterface) const override;
    KisConfigWidget *createConfigurationWidget(QWidget *parent, const KisPaintDeviceSP dev, bool useForMasks) const override;

private:
    mutable KisCachedSelection m_selectionsCache;
    mutable KisCachedPaintDevice m_grayDevicesCache;
    mutable KisCachedPaintDevice m_genericDevicesCache;

    static QVector<quint8> makeHardnessLut(qreal hardness);
    static QVector<quint8> makeNoiseWeightLut(qreal hardness);
    
    static inline quint8 mapU8ToRange(quint8 value, quint8 new_min, quint8 new_max) {
        Q_UNUSED(new_min);
        Q_UNUSED(new_max);
        return value;
    }
    template <typename T>
    static inline T mapU8ToRange(quint8 value, T new_min, T new_max) {
        return value * (new_max - new_min) / 255 + new_min;
    }

    KisPaintDeviceSP makeGeneratorPaintDevice(KisPaintDeviceSP prototype,
                                              const QString & prefix,
                                              const QRect &applyRect,
                                              const KisHalftoneFilterConfiguration *config,
                                              KoUpdater *progressUpdater) const;

    bool checkUpdaterInterruptedAndSetPercent(KoUpdater *progressUpdater, int percent) const;
    
    void processIntensity(KisPaintDeviceSP device,
                          const QRect& applyRect,
                          const KisHalftoneFilterConfiguration *config,
                          KoUpdater *progressUpdater) const;
    template <typename ChannelType>
    void processChannel(KisPaintDeviceSP device,
                        KisPaintDeviceSP generatorDevice,
                        const QRect &applyRect,
                        const KisHalftoneFilterConfiguration *config,
                        const QString & prefix,
                        KoChannelInfo * channelInfo) const;
    void processChannels(KisPaintDeviceSP device,
                         const QRect& applyRect,
                         const KisHalftoneFilterConfiguration *config,
                         KoUpdater *progressUpdater) const;
    void processAlpha(KisPaintDeviceSP device,
                      const QRect& applyRect,
                      const KisHalftoneFilterConfiguration *config,
                      KoUpdater *progressUpdater) const;
    void processMask(KisPaintDeviceSP device,
                     const QRect& applyRect,
                     const KisHalftoneFilterConfiguration *config,
                     KoUpdater *progressUpdater) const;

};

#endif
