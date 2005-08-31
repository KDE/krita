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

#ifndef _KIS_CONVOLUTION_FILTER_H_
#define _KIS_CONVOLUTION_FILTER_H_

#include "kis_filter.h"
#include "kis_matrix.h"
#include "kis_view.h"
#include <kdebug.h>

class KisConvolutionConfiguration : public KisFilterConfiguration {
public:
    KisConvolutionConfiguration(KisMatrix3x3* matrixes) : m_matrixes(matrixes) {};
public:
    inline KisMatrix3x3* matrixes() { return m_matrixes; };
private:
    KisMatrix3x3* m_matrixes;
};


class KisConvolutionFilter : public KisFilter {
    Q_OBJECT
public:
    KisConvolutionFilter(const KisID& id, const QString & category, const QString & entry);
public:
    virtual void process(KisPaintDeviceImplSP,KisPaintDeviceImplSP, KisFilterConfiguration* , const QRect&);
    virtual bool supportsIncrementalPainting() { return false; }
};


/** 
 * This class is used for a convolution filter with a constant matrix
 */
class KisConvolutionConstFilter : public KisConvolutionFilter {
public:
    KisConvolutionConstFilter(const KisID& id, const QString & category, const QString & entry) : KisConvolutionFilter(id, category, entry) { } ;
    virtual ~KisConvolutionConstFilter();
public:
    virtual KisFilterConfiguration* configuration(QWidget*, KisPaintDeviceImplSP dev);
protected:
    KisMatrix3x3* m_matrixes;
};

#endif
