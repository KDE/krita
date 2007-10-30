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
#include "kis_filter_registry.h"
#include "kis_filter_configuration.h"
#include "kis_convolution_painter.h"

class KisConvolutionFilter : public KisFilter {

public:
    KisConvolutionFilter(const KoID& id, const KoID & category, const QString & entry)
        : KisFilter( id, category, entry )
        {}
public:
    void process(KisFilterConstantProcessingInformation src,
                 KisFilterProcessingInformation dst,
                 const QSize& size,
                 const KisFilterConfiguration* config,
                 KoUpdater* progressUpdater = 0
        ) const;
    virtual bool supportsIncrementalPainting() const { return false; }
    virtual ColorSpaceIndependence colorSpaceIndependence() const { return FULLY_INDEPENDENT; }
    virtual int overlapMarginNeeded(KisFilterConfiguration* c) const;

protected:

    KisKernelSP m_matrix;
};

#endif
