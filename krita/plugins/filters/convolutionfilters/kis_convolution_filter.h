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
#include "kis_filter_configuration.h"
#include "kis_convolution_painter.h"

class KisConvolutionConfiguration : public KisFilterConfiguration {
public:
    KisConvolutionConfiguration(const QString & name, KisKernel * matrix)
        : KisFilterConfiguration( name, 1 )
        , m_matrix(matrix)
    {};

    void fromXML(const QString & s);
    QString toString();

public:

    inline KisKernelSP matrix() { return m_matrix; };

private:

    KisKernelSP m_matrix;

};


class KisConvolutionFilter : public KisFilter {

    Q_OBJECT

public:

    KisConvolutionFilter(const KoID& id, const QString & category, const QString & entry)
        : KisFilter( id, category, entry )
        {};

public:

    virtual void process(KisPaintDeviceSP,KisPaintDeviceSP, KisFilterConfiguration* , const QRect&);
    virtual bool supportsIncrementalPainting() { return false; }
    virtual ColorSpaceIndependence colorSpaceIndependence() { return FULLY_INDEPENDENT; };


};


/**
 * This class is used for a convolution filter with a constant matrix
 */
class KisConvolutionConstFilter : public KisConvolutionFilter {

public:

    KisConvolutionConstFilter(const KoID& id, const QString & category, const QString & entry) 
	: KisConvolutionFilter(id, category, entry) 
    {};

    virtual ~KisConvolutionConstFilter() {};

public:

    virtual KisFilterConfiguration * configuration(QWidget*);
    virtual KisFilterConfiguration * configuration() { return configuration(0); };

protected:

    KisKernelSP m_matrix;
};

#endif
