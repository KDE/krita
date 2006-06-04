/*
 * This file is part of the KDE project
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

#ifndef _KIS_PIXELIZE_FILTER_H_
#define _KIS_PIXELIZE_FILTER_H_

#include "kis_filter.h"
#include "kis_filter_config_widget.h"

class KisPixelizeFilterConfiguration : public KisFilterConfiguration
{
public:
    KisPixelizeFilterConfiguration(quint32 pixelWidth, quint32 pixelHeight)
        : KisFilterConfiguration( "pixelize", 1 )
        {
            setProperty("pixelWidth", pixelWidth);
            setProperty("pixelHeight", pixelHeight);
        };
public:
    inline quint32 pixelWidth() { return getInt("pixelWidth"); };
    inline quint32 pixelHeight() {return getInt("pixelHeight"); };
};

class KisPixelizeFilter : public KisFilter
{
public:
    KisPixelizeFilter();
public:
    virtual void process(KisPaintDeviceSP,KisPaintDeviceSP, KisFilterConfiguration* , const QRect&);
    static inline KoID id() { return KoID("pixelize", i18n("Pixelize")); };
    virtual bool supportsPainting() { return true; }
    virtual bool supportsPreview() { return true; }
    virtual std::list<KisFilterConfiguration*> listOfExamplesConfiguration(KisPaintDeviceSP )
        { std::list<KisFilterConfiguration*> list; list.insert(list.begin(), new KisPixelizeFilterConfiguration(10,10)); return list; }
public:
    virtual KisFilterConfigWidget * createConfigurationWidget(QWidget* parent, KisPaintDeviceSP dev);
    virtual KisFilterConfiguration* configuration(QWidget*);
    virtual KisFilterConfiguration * configuration();
private:
    void pixelize(KisPaintDeviceSP src, KisPaintDeviceSP dst, int x, int y, int w, int h, int pixelWidth, int pixelHeight);
};

#endif
