/*
 * This file is part of Krita
 *
 * Copyright (c) Michael Thaler <michael.thaler@physik.tu-muenchen.de>
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

#ifndef _KIS_RAINDROPS_FILTER_H_
#define _KIS_RAINDROPS_FILTER_H_

#include "kis_filter.h"
#include "kis_filter_config_widget.h"

class KisRainDropsFilterConfiguration : public KisFilterConfiguration
{
public:
    KisRainDropsFilterConfiguration(Q_UINT32 dropSize, Q_UINT32 number, Q_UINT32 fishEyes)
        : KisFilterConfiguration( "raindrops", 1 )
        {
            setProperty("dropsize", dropSize);
            setProperty("number", number);
            setProperty("fishEyes", fishEyes);
        };
public:
    inline Q_UINT32 dropSize() { return getInt("dropsize"); };
    inline Q_UINT32 number() {return getInt("number"); };
    inline Q_UINT32 fishEyes() {return getInt("fishEyes"); };
    
};

class KisRainDropsFilter : public KisFilter
{
public:
    KisRainDropsFilter();
public:
    virtual void process(KisPaintDeviceSP,KisPaintDeviceSP, KisFilterConfiguration* , const QRect&);
    static inline KisID id() { return KisID("raindrops", i18n("Raindrops")); };
    virtual bool supportsPainting() { return false; }
    virtual bool supportsPreview() { return true; }
    virtual std::list<KisFilterConfiguration*> listOfExamplesConfiguration(KisPaintDeviceSP )
    { std::list<KisFilterConfiguration*> list; list.insert(list.begin(), new KisRainDropsFilterConfiguration( 30, 80, 20)); return list; }

public:
    virtual KisFilterConfigWidget * createConfigurationWidget(QWidget* parent, KisPaintDeviceSP dev);
    virtual KisFilterConfiguration* configuration(QWidget*);
private:
    void   rainDrops(KisPaintDeviceSP src, KisPaintDeviceSP dst, const QRect& rect, int DropSize, int Amount, int Coeff);
    bool** CreateBoolArray (uint Columns, uint Rows);
    void   FreeBoolArray (bool** lpbArray, uint Columns);
    uchar  LimitValues (int ColorValue);
};

#endif
