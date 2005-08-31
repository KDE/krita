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

/**
@author Cyrille Berger
*/

class KisSimpleNoiseReducerConfiguration
    : public KisFilterConfiguration
{
    public:
        KisSimpleNoiseReducerConfiguration(int nt, int ws) : threshold(nt), windowsize(ws) { }
        int threshold;
        int windowsize;
};

class KisSimpleNoiseReducer : public KisFilter
{
    public:
        KisSimpleNoiseReducer();
        ~KisSimpleNoiseReducer();
    public:
        virtual void process(KisPaintDeviceImplSP src, KisPaintDeviceImplSP dst, KisFilterConfiguration*, const QRect&);
        virtual KisFilterConfiguration* configuration(QWidget* nwidget, KisPaintDeviceImplSP dev);
        virtual KisFilterConfigWidget * createConfigurationWidget(QWidget* parent, KisPaintDeviceImplSP dev);

        static inline KisID id() { return KisID("simplenoisereducer", i18n("Simple Noise Reduceer")); };
        virtual bool supportsPainting() { return true; }
        virtual bool supportsPreview() { return true; }
        virtual bool supportsIncrementalPainting() { return false; }
};

#endif
