/*
 * This file is part of Krita
 *
 * Copyright (c) 2006 Frederic Coiffier <fcoiffie@gmail.com>
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

#ifndef _KIS_LEVEL_FILTER_H_
#define _KIS_LEVEL_FILTER_H_

#include "kis_filter.h"
#include "kis_filter_config_widget.h"

class WdgLevel;
class QWidget;
class KisColorAdjustment;
class KisHistogram;

class KisLevelFilterConfiguration : public KisFilterConfiguration {
    
public:
    
    KisLevelFilterConfiguration();
    virtual ~KisLevelFilterConfiguration();
    virtual void fromXML( const QString&  );
    virtual QString toString();
    
public:
    Q_UINT8 blackvalue, whitevalue;
    double gammavalue;
    Q_UINT16 outblackvalue, outwhitevalue;
    KisColorAdjustment * m_adjustment;
};

/**
 * This class affect Intensity Y of the image
 */
class KisLevelFilter : public KisFilter
{

public:

    KisLevelFilter();

public:

    virtual KisFilterConfigWidget * createConfigurationWidget(QWidget* parent, KisPaintDeviceSP dev);
    virtual KisFilterConfiguration * configuration(QWidget *);
    virtual KisFilterConfiguration * configuration() { return new KisLevelFilterConfiguration(); };
    virtual void process(KisPaintDeviceSP, KisPaintDeviceSP, KisFilterConfiguration* , const QRect&);
    static inline KisID id() { return KisID("levels", i18n("Levels")); };
    virtual bool supportsPainting() { return true; }
    virtual bool supportsPreview() { return true; }
    virtual bool supportsIncrementalPainting() { return false; }
    virtual std::list<KisFilterConfiguration*> listOfExamplesConfiguration(KisPaintDeviceSP dev);

    virtual ColorSpaceIndependence colorSpaceIndependence() { return TO_LAB16; };
    virtual bool workWith(KisColorSpace* cs);
};


class KisLevelConfigWidget : public KisFilterConfigWidget {
Q_OBJECT
public:
    KisLevelConfigWidget(QWidget * parent, KisPaintDeviceSP dev, const char * name = 0, WFlags f = 0 );
    virtual ~KisLevelConfigWidget();

    KisLevelFilterConfiguration * config();
    void setConfiguration( KisFilterConfiguration * config );
    WdgLevel * m_page;

protected slots:
    void drawHistogram(bool logarithmic = false);

protected:
    KisHistogram *histogram;
    bool m_histlog;
};

#endif
