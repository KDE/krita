/*
  a *  SPDX-FileCopyrightText: 2005 Cyrille Berger <cberger@cberger.net>
  *
  *  SPDX-License-Identifier: GPL-2.0-or-later
  */
#ifndef KISSIMPLENOISEREDUCER_H
#define KISSIMPLENOISEREDUCER_H

#include <filter/kis_filter.h>
#include "kis_config_widget.h"
/**
   @author Cyrille Berger
*/

class KisSimpleNoiseReducer : public KisFilter
{
public:
    KisSimpleNoiseReducer();
    ~KisSimpleNoiseReducer() override;
public:

    void processImpl(KisPaintDeviceSP device,
                     const QRect& applyRect,
                     const KisFilterConfigurationSP config,
                     KoUpdater* progressUpdater
                     ) const override;
    KisConfigWidget * createConfigurationWidget(QWidget* parent, const KisPaintDeviceSP dev, bool useForMasks) const override;

    static inline KoID id() {
        return KoID("gaussiannoisereducer", i18n("Gaussian Noise Reducer"));
    }

    QRect changedRect(const QRect &rect, const KisFilterConfigurationSP _config, int lod) const override;
    QRect neededRect(const QRect &rect, const KisFilterConfigurationSP _config, int lod) const override;

protected:
    KisFilterConfigurationSP  defaultConfiguration(KisResourcesInterfaceSP resourcesInterface) const override;
};

#endif
