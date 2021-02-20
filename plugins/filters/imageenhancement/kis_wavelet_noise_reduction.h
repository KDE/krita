/*
 * This file is part of the KDE project
 *
 * SPDX-FileCopyrightText: 2005 Cyrille Berger <cberger@cberger.net>
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_WAVELET_NOISE_REDUCTION_H
#define KIS_WAVELET_NOISE_REDUCTION_H

#include <vector>

#include <filter/kis_filter.h>

#define BEST_WAVELET_THRESHOLD_VALUE 7.0

/**
@author Cyrille Berger
*/
class KisWaveletNoiseReduction : public KisFilter
{
public:
    KisWaveletNoiseReduction();

    ~KisWaveletNoiseReduction() override;

public:

    void processImpl(KisPaintDeviceSP device,
                     const QRect& applyRect,
                     const KisFilterConfigurationSP config,
                     KoUpdater* progressUpdater
                     ) const override;
    KisConfigWidget * createConfigurationWidget(QWidget* parent, const KisPaintDeviceSP dev, bool useForMasks) const override;

    static inline KoID id() {
        return KoID("waveletnoisereducer", i18n("Wavelet Noise Reducer"));
    }

private:
    KisFilterConfigurationSP  defaultConfiguration(KisResourcesInterfaceSP resourcesInterface) const override;
};

#endif
