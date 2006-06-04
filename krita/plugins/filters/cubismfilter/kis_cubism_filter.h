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

#ifndef _KIS_CUBISM_FILTER_H_
#define _KIS_CUBISM_FILTER_H_

#include "kis_filter.h"
#include "kis_filter_config_widget.h"
#include <kdebug.h>

class KisPolygon;

class KisCubismFilterConfiguration : public KisFilterConfiguration
{
public:
    KisCubismFilterConfiguration(quint32 tileSize, quint32 tileSaturation)
        : KisFilterConfiguration( "cubism", 1 )
        , m_tileSize(tileSize)
        , m_tileSaturation(tileSaturation)
    {
        setProperty("tileSize", tileSize);
        setProperty("tileSaturation", tileSaturation);
    };
public:
    inline quint32 tileSize() { return getInt("tileSize"); };
    inline quint32 tileSaturation() {return getInt("tileSaturation"); };
private:
    quint32 m_tileSize;
    quint32 m_tileSaturation;
};

class KisCubismFilter : public KisFilter
{
public:
    KisCubismFilter();
public:
    virtual void process(KisPaintDeviceSP,KisPaintDeviceSP, KisFilterConfiguration* , const QRect&);
    static inline KoID id() { return KoID("cubism", i18n("Cubism")); };
    virtual bool supportsPainting() { return false; }
    virtual bool supportsPreview() { return true; }
    virtual std::list<KisFilterConfiguration*> listOfExamplesConfiguration(KisPaintDeviceSP )
    { std::list<KisFilterConfiguration*> list; list.insert(list.begin(), new KisCubismFilterConfiguration(10,10)); return list; }
    virtual bool workWith(KoColorSpace* cs);
    virtual ColorSpaceIndependence colorSpaceIndependence() { return TO_RGBA8; };
public:
    virtual KisFilterConfigWidget * createConfigurationWidget(QWidget* parent, KisPaintDeviceSP dev);
    virtual KisFilterConfiguration* configuration(QWidget*);
    virtual KisFilterConfiguration* configuration() { return new KisCubismFilterConfiguration( 10, 10); };
private:
    //this function takes an array of ordered indices i1,i2,i3,... and randomizes them i3,i1,i2,...
        void randomizeIndices (qint32 count, qint32* indices);
        qint32 randomIntNumber(qint32 lowestNumber, qint32 highestNumber);
        double randomDoubleNumber(double lowestNumber, double highestNumber);
        double   calcAlphaBlend(double *vec, double oneOverDist, double x, double y);
        void convertSegment(qint32 x1, qint32 y1, qint32 x2, qint32  y2, qint32 offset, qint32* min, qint32* max, qint32 xmin, qint32 xmax);
        void fillPolyColor(KisPaintDeviceSP src, KisPaintDeviceSP dst, KisPolygon* poly, const quint8* col, quint8* dest, QRect rect);
        void cubism(KisPaintDeviceSP src, KisPaintDeviceSP dst, const QRect& rect, quint32 tileSize, quint32 tileSaturation);

};

#endif
