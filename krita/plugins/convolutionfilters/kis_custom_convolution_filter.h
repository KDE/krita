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
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#ifndef _KIS_CUSTOM_CONVOLUTION_FILTER_H_
#define _KIS_CUSTOM_CONVOLUTION_FILTER_H_

#include "kis_convolution_filter.h"

class KisCustomConvolutionConfiguration : public KisConvolutionConfiguration {
public:
	KisCustomConvolutionConfiguration(KisMatrix3x3* matrixes) : KisConvolutionConfiguration(matrixes) {
	};
	~KisCustomConvolutionConfiguration() { delete m_matrixes; };
private:
	KisMatrix3x3* m_matrixes;
};

class KisCustomConvolutionFilter : public KisConvolutionFilter {
public:
	KisCustomConvolutionFilter(KisView * view);
public:
	static inline KisID id() { return KisID("custom convolution", i18n("Custom Convolution")); };
	virtual bool supportsPainting() { return false; }

public:
	virtual KisFilterConfigurationWidget* createConfigurationWidget(QWidget* parent);
	virtual KisFilterConfiguration* configuration(KisFilterConfigurationWidget*);
protected:
	virtual KisMatrix3x3* matrixes() { return m_matrix; };
private:
	KisMatrix3x3* m_matrix;
};




#endif
