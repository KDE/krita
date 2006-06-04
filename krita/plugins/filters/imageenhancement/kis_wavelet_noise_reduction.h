/*
 * This file is part of the KDE project
 *
 * Copyright (c) 2005 Cyrille Berger <cberger@cberger.net>
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

#ifndef KIS_WAVELET_NOISE_REDUCTION_H
#define KIS_WAVELET_NOISE_REDUCTION_H

#include <vector>

#include <kis_filter.h>

#define BEST_WAVELET_THRESHOLD_VALUE 7.0

class KisWaveletNoiseReductionConfiguration
    : public KisFilterConfiguration
{
public:
    KisWaveletNoiseReductionConfiguration(double nt)
        : KisFilterConfiguration( "waveletnoisereducer",  1 )
    {
           setProperty("threshold", nt);
    }
        
    double threshold() { return getDouble("threshold"); };
};


/**
@author Cyrille Berger
*/
class KisWaveletNoiseReduction : public KisFilter
{
public:
    KisWaveletNoiseReduction();

    ~KisWaveletNoiseReduction();

public:
    virtual void process(KisPaintDeviceSP src, KisPaintDeviceSP dst, KisFilterConfiguration*, const QRect&);
    virtual KisFilterConfiguration * configuration(QWidget* nwidget);
    virtual KisFilterConfiguration * configuration() {return new KisWaveletNoiseReductionConfiguration( BEST_WAVELET_THRESHOLD_VALUE );};
    virtual KisFilterConfigWidget * createConfigurationWidget(QWidget* parent, KisPaintDeviceSP dev);

    static inline KoID id() { return KoID("waveletnoisereducer", i18n("Wavelet Noise Reducer")); };
    virtual bool supportsPainting() { return false; }
    virtual bool supportsPreview() { return true; }
    virtual bool supportsIncrementalPainting() { return false; }
    virtual bool supportsThreading() { return false; };

};

#endif
