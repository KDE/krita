/*
 * This file is part of the KDE project
 *
 * Copyright (c) 2005-2006 Cyrille Berger <cberger@cberger.net>
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

#ifndef KIS_HALFTONE_REDUCTION_H_
#define KIS_HALFTONE_REDUCTION_H_

#include <vector>

#include <kis_filter.h>

#define BEST_WAVELET_FREQUENCY_VALUE 2

    
#include <kparts/plugin.h>

class KritaHalftone : public KParts::Plugin
{
  public:
        KritaHalftone(QObject *parent, const char *name, const QStringList &);
        virtual ~KritaHalftone();
};


class KisHalftoneReductionConfiguration
    : public KisFilterConfiguration
{
public:
    KisHalftoneReductionConfiguration(double nt, int hs)
        : KisFilterConfiguration( "halftone",  1 )
    {
           setProperty("frequency", nt);
           setProperty("halfsize", hs);
    }
        
};


/**
@author Cyrille Berger
*/
class KisHalftoneReduction : public KisFilter
{
public:
    KisHalftoneReduction();

    ~KisHalftoneReduction();

public:
    virtual void process(KisPaintDeviceSP src, KisPaintDeviceSP dst, KisFilterConfiguration*, const QRect&);
    virtual KisFilterConfiguration * configuration(QWidget* nwidget);
    virtual KisFilterConfiguration * configuration() {return new KisHalftoneReductionConfiguration( BEST_WAVELET_FREQUENCY_VALUE, 2 );};
    virtual KisFilterConfigWidget * createConfigurationWidget(QWidget* parent, KisPaintDeviceSP dev);

    static inline KisID id() { return KisID("halftone", i18n("Halftone Reducer")); };
    virtual bool supportsPainting() { return false; }
    virtual bool supportsPreview() { return true; }
    virtual bool supportsIncrementalPainting() { return false; }
    virtual bool supportsThreading() { return false; };
    virtual bool supportsAdjustmentLayers() { return false; }

};

#endif
