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
#include "kis_view.h"

class KisView;

class KisGaussianBlurFilter : public KisConvolutionFilter {
public:
	KisGaussianBlurFilter(KisView * view);
public:
	virtual KisMatrix3x3* matrixes();
	static inline QString name() { return "Gaussian blur"; };
};

class KisSharpenFilter : public KisConvolutionFilter {
public:
	KisSharpenFilter(KisView * view);
public:
	virtual KisMatrix3x3* matrixes();
	static inline QString name() { return "Sharpen"; };
};

class KisMeanRemovalFilter : public KisConvolutionFilter {
public:
	KisMeanRemovalFilter(KisView * view);
public:
	virtual KisMatrix3x3* matrixes();
	static inline QString name() { return "Mean Removal"; };
};

class KisEmbossLaplascianFilter : public KisConvolutionFilter {
public:
	KisEmbossLaplascianFilter(KisView * view);
public:
	virtual KisMatrix3x3* matrixes();
	static inline QString name() { return "Emboss laplascian"; };
};

class KisEmbossInAllDirectionsFilter : public KisConvolutionFilter {
public:
	KisEmbossInAllDirectionsFilter(KisView * view);
public:
	virtual KisMatrix3x3* matrixes();
	static inline QString name() { return "Emboss in all directions"; };
};

class KisEmbossHorizontalVerticalFilter : public KisConvolutionFilter {
public:
	KisEmbossHorizontalVerticalFilter(KisView * view);
public:
	virtual KisMatrix3x3* matrixes();
	static inline QString name() { return "Emboss horizontal and vertical"; };
};

class KisEmbossVerticalFilter : public KisConvolutionFilter {
public:
	KisEmbossVerticalFilter(KisView * view);
public:
	virtual KisMatrix3x3* matrixes();
	static inline QString name() { return "Emboss vertical only"; };
};

class KisEmbossHorizontalFilter : public KisConvolutionFilter {
public:
	KisEmbossHorizontalFilter(KisView * view);
public:
	virtual KisMatrix3x3* matrixes();
	static inline QString name() { return "Emboss horizontal only"; };
};

class KisEmbossDiagonalFilter : public KisConvolutionFilter {
public:
	KisEmbossDiagonalFilter(KisView * view);
public:
	virtual KisMatrix3x3* matrixes();
	static inline QString name() { return "Emboss diagonal"; };
};

class KisTopEdgeDetectionFilter : public KisConvolutionFilter {
public:
	KisTopEdgeDetectionFilter(KisView * view);
public:
	virtual KisMatrix3x3* matrixes();
	static inline QString name() { return "Top Edge detections"; };
};

class KisRightEdgeDetectionFilter : public KisConvolutionFilter {
public:
	KisRightEdgeDetectionFilter(KisView * view);
public:
	virtual KisMatrix3x3* matrixes();
	static inline QString name() { return "Right Edge detections"; };
};

class KisBottomEdgeDetectionFilter : public KisConvolutionFilter {
public:
	KisBottomEdgeDetectionFilter(KisView * view);
public:
	virtual KisMatrix3x3* matrixes();
	static inline QString name() { return "Bottom Edge detections"; };
};

class KisLeftEdgeDetectionFilter : public KisConvolutionFilter {
public:
	KisLeftEdgeDetectionFilter(KisView * view);
public:
	virtual KisMatrix3x3* matrixes();
	static inline QString name() { return "Left Edge detections"; };
};


class KritaConvolutionFilters : public KParts::Plugin
{
public:
	KritaConvolutionFilters(QObject *parent, const char *name, const QStringList &);
	virtual ~KritaConvolutionFilters();
};

#endif
