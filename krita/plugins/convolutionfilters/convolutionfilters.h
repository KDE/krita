/* This file is part of the KDE project
   Copyright (c) 2004 Cyrille Berger <cberger@cberger.net>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#ifndef EXAMPLE_H
#define EXAMPLE_H

#include <kparts/plugin.h>
#include "kis_convolution_filter.h"
#include "kis_matrix.h"

class KisView;

class KisGaussianBlurFilter : public KisConvolutionFilter {
public:
	KisGaussianBlurFilter();
public:
	virtual KisMatrix3x3* matrixes();
};

class KisSharpenFilter : public KisConvolutionFilter {
public:
	KisSharpenFilter();
public:
	virtual KisMatrix3x3* matrixes();
};

class KisMeanRemovalFilter : public KisConvolutionFilter {
public:
	KisMeanRemovalFilter();
public:
	virtual KisMatrix3x3* matrixes();
};

class KisEmbossLaplascianFilter : public KisConvolutionFilter {
public:
	KisEmbossLaplascianFilter();
public:
	virtual KisMatrix3x3* matrixes();
};

class KisEmbossInAllDirectionsFilter : public KisConvolutionFilter {
public:
	KisEmbossInAllDirectionsFilter();
public:
	virtual KisMatrix3x3* matrixes();
};

class KisEmbossHorizontalVerticalFilter : public KisConvolutionFilter {
public:
	KisEmbossHorizontalVerticalFilter();
public:
	virtual KisMatrix3x3* matrixes();
};

class KisEmbossVerticalFilter : public KisConvolutionFilter {
public:
	KisEmbossVerticalFilter();
public:
	virtual KisMatrix3x3* matrixes();
};

class KisEmbossHorizontalFilter : public KisConvolutionFilter {
public:
	KisEmbossHorizontalFilter();
public:
	virtual KisMatrix3x3* matrixes();
};

class KisEmbossDiagonalFilter : public KisConvolutionFilter {
public:
	KisEmbossDiagonalFilter();
public:
	virtual KisMatrix3x3* matrixes();
};

class KisTopEdgeDetectionFilter : public KisConvolutionFilter {
public:
	KisTopEdgeDetectionFilter();
public:
	virtual KisMatrix3x3* matrixes();
};

class KritaConvolutionFilters : public KParts::Plugin
{
public:
	KritaConvolutionFilters(QObject *parent, const char *name, const QStringList &);
	virtual ~KritaConvolutionFilters();
};

#endif
