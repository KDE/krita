/*
 * KDE. Krita Project.
 *
 * Copyright (c) 2020 Deif Lou <ginoba@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef KIS_HALFTONE_FILTER_H
#define KIS_HALFTONE_FILTER_H

#include <QObject>
#include <QVector>

#include <filter/kis_filter.h>
#include <kis_filter_configuration.h>

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
    static KisPaintDeviceSP makeGeneratorPaintDevice(const QString & prefix,
                                                     const QRect &applyRect,
                                                     const KisHalftoneFilterConfiguration *config,
                                                     KoUpdater *progressUpdater);
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
