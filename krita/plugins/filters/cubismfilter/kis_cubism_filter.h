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

#include "filter/kis_filter.h"
#include "kis_config_widget.h"
#include <kis_debug.h>

class KisPolygon;

class KisCubismFilter : public KisFilter
{
public:
    KisCubismFilter();
public:
    using KisFilter::process;

    void process(KisConstProcessingInformation src,
                 KisProcessingInformation dst,
                 const QSize& size,
                 const KisFilterConfiguration* config,
                 KoUpdater* progressUpdater
                ) const;
    static inline KoID id() {
        return KoID("cubism", i18n("Cubism"));
    }

    virtual bool workWith(const KoColorSpace* cs) const;
public:
    virtual KisConfigWidget * createConfigurationWidget(QWidget* parent, const KisPaintDeviceSP dev, const KisImageWSP image = 0) const;
protected:
    virtual KisFilterConfiguration* factoryConfiguration(const KisPaintDeviceSP) const;
private:
    //this function takes an array of ordered indices i1,i2,i3,... and randomizes them i3,i1,i2,...
    void randomizeIndices(qint32 count, qint32* indices) const;
    qint32 randomIntNumber(qint32 lowestNumber, qint32 highestNumber)  const;
    double randomDoubleNumber(double lowestNumber, double highestNumber)  const;
    double   calcAlphaBlend(double *vec, double oneOverDist, double x, double y) const;
    void convertSegment(qint32 x1, qint32 y1, qint32 x2, qint32  y2, qint32 offset, qint32* min, qint32* max, qint32 xmin, qint32 xmax) const;
    void fillPolyColor(KisPaintDeviceSP src, const QPoint& srcTopLeft, KisPaintDeviceSP dst, const QPoint & dstTopLeft, const QSize& size, KisPolygon* poly, const quint8* col, quint8* dest) const;
    void cubism(KisPaintDeviceSP src, const QPoint& srcTopLeft, KisPaintDeviceSP dst, const QPoint & dstTopLeft, const QSize& size, quint32 tileSize, quint32 tileSaturation) const;

};

#endif
