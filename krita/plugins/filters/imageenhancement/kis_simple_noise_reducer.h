/*
 *  Copyright (c) 2005 Cyrille Berger <cberger@cberger.net>
 *
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
#ifndef KISSIMPLENOISEREDUCER_H
#define KISSIMPLENOISEREDUCER_H

#include <kis_filter.h>
#include "kis_filter_config_widget.h"
/**
@author Cyrille Berger
*/

class KisSimpleNoiseReducerConfiguration
    : public KisFilterConfiguration
{
    public:
        KisSimpleNoiseReducerConfiguration(int nt, int ws)
    : KisFilterConfiguration( "gaussiannoisereducer", 1 )
        {
            setProperty("threshold", nt);
            setProperty("windowsize", ws);
        }
        int threshold() { return getInt("threshold"); };
        int windowsize() { return getInt("windowsize"); };
};

class KisSimpleNoiseReducer : public KisFilter
{
    public:
        KisSimpleNoiseReducer();
        ~KisSimpleNoiseReducer();
    public:
        virtual void process(KisPaintDeviceSP src, KisPaintDeviceSP dst, KisFilterConfiguration*, const QRect&);
        virtual KisFilterConfiguration * configuration(QWidget* nwidget);
        virtual KisFilterConfiguration * configuration() { return new KisSimpleNoiseReducerConfiguration( 50, 1); };
        virtual KisFilterConfigWidget * createConfigurationWidget(QWidget* parent, KisPaintDeviceSP dev);

        static inline KoID id() { return KoID("gaussiannoisereducer", i18n("Gaussian Noise Reducer")); };
        virtual bool supportsPainting() { return false; }
        virtual bool supportsPreview() { return true; }
        virtual bool supportsIncrementalPainting() { return false; }
};

#endif
