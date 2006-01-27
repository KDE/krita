/*
 * This file is part of Krita
 *
 * Copyright (c) 2004 Cyrille Berger <cberger@cberger.net>
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

#ifndef _KIS_PERCHANNEL_FILTER_H_
#define _KIS_PERCHANNEL_FILTER_H_

#include <qpair.h>
#include <qptrlist.h>
#include "kis_filter.h"
#include "kis_filter_config_widget.h"

class WdgPerChannel;

class KisPerChannelFilterConfiguration
    : public KisFilterConfiguration
{
public:
    KisPerChannelFilterConfiguration(int n);
    ~KisPerChannelFilterConfiguration();

public:
    Q_UINT16 *transfers[256];

private:
    Q_UINT16 m_nTransfers;
};


/**
 * This class is generic for filters that affect channel separately
 */
class KisPerChannelFilter
    : public KisFilter
{
public:
    KisPerChannelFilter();
public:
    virtual KisFilterConfigWidget * createConfigurationWidget(QWidget* parent, KisPaintDeviceSP dev);
    virtual KisFilterConfiguration* configuration(QWidget*);
    virtual void process(KisPaintDeviceSP, KisPaintDeviceSP, KisFilterConfiguration* , const QRect&);
    static inline KisID id() { return KisID("perchannel", i18n("Color Adjustment")); };
    virtual bool supportsPainting() { return true; }
    virtual bool supportsPreview() { return true; }
    virtual std::list<KisFilterConfiguration*> listOfExamplesConfiguration(KisPaintDeviceSP dev);

    virtual ColorSpaceIndependence colorSpaceIndendendence() { return FULLY_INDEPENDENT; };
private:
};

class KisPerChannelConfigWidget : public KisFilterConfigWidget {

    typedef KisFilterConfigWidget super;
    Q_OBJECT

public:
    KisPerChannelConfigWidget(QWidget * parent, KisPaintDeviceSP dev, const char * name = 0, WFlags f = 0 );
    virtual ~KisPerChannelConfigWidget() {};

    KisPerChannelFilterConfiguration * config();

private slots:
    virtual void setActiveChannel(int ch);

private:
    WdgPerChannel * m_page;
    KisPaintDeviceSP m_dev;
    KisHistogram *m_histogram;
    QPtrList<QPair<double,double> > *m_curves;
    int m_activeCh;
};

#endif
