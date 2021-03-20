/*
 * This file is part of Krita
 *
 * SPDX-FileCopyrightText: 2006 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef COLORTRANSFER_H
#define COLORTRANSFER_H

#include <QObject>
#include <QVariant>
#include <filter/kis_filter.h>

class FastColorTransferPlugin : public QObject
{
    Q_OBJECT
public:
    FastColorTransferPlugin(QObject *parent, const QVariantList &);
    ~FastColorTransferPlugin() override;
};

class KisFilterFastColorTransfer : public KisFilter
{
public:
    KisFilterFastColorTransfer();
public:

    void processImpl(KisPaintDeviceSP device,
                     const QRect& applyRect,
                     const KisFilterConfigurationSP config,
                     KoUpdater* progressUpdater) const override;
    static inline KoID id() {
        return KoID("colortransfer", i18n("Color Transfer"));
    }

public:
    KisConfigWidget * createConfigurationWidget(QWidget* parent, const KisPaintDeviceSP dev, bool useForMasks) const override;
    KisFilterConfigurationSP defaultConfiguration(KisResourcesInterfaceSP resourcesInterface) const override;
};

#endif
