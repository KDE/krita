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

#ifndef _KIS_CUSTOM_CONVOLUTION_FILTER_H_
#define _KIS_CUSTOM_CONVOLUTION_FILTER_H_

#include "kis_convolution_filter.h"
#include "kis_filter_config_widget.h"
#include "KoID.h"
#include "kis_types.h"

class QWidget;

class KisCustomConvolutionFilter : public KisConvolutionFilter {

public:
    KisCustomConvolutionFilter() : KisConvolutionFilter(id(), "enhance", i18n("&Custom Convolution...")) {};
    
public:
    static inline KoID id() { return KoID("custom convolution", i18n("Custom Convolution")); };
    virtual bool supportsPainting() { return true; }
    virtual bool supportsIncrementalPainting() { return true; }

public:
    virtual KisFilterConfigWidget * createConfigurationWidget(QWidget* parent, KisPaintDeviceSP dev);
    virtual KisFilterConfiguration * configuration(QWidget*);
    virtual KisFilterConfiguration * configuration() { return configuration(0); }; 
protected:
    virtual KisKernelSP matrix() { return m_matrix; };
private:
    KisKernelSP m_matrix;
};




#endif
