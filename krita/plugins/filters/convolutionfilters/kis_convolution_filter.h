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

#include "filter/kis_filter.h"
#include "filter/kis_filter_registry.h"
#include "filter/kis_filter_configuration.h"
#include "kis_convolution_painter.h"

class KisConvolutionFilter : public KisFilter
{

public:
    KisConvolutionFilter(const KoID& id, const KoID & category, const QString & entry);
public:
    using KisFilter::process;

    void processImpl(KisPaintDeviceSP device,
                     const QRect& applyRect,
                     const KisFilterConfiguration* config,
                     KoUpdater* progressUpdater) const;
    virtual int overlapMarginNeeded(const KisFilterConfiguration* c) const;
protected:
    void setIgnoreAlpha(bool v);

protected:
    KisConvolutionKernelSP m_matrix;
    bool m_ignoreAlpha;
};

#endif
