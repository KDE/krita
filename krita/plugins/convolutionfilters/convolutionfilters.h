/*
 * This file is part of Krita
 *
 * Copyright (c) 2004 Cyrille Berger <cberger@cberger.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
*/

#ifndef EXAMPLE_H
#define EXAMPLE_H

#include <kparts/plugin.h>
#include "kis_convolution_filter.h"
#include "kis_matrix.h"
#include "kis_view.h"

class KisView;

// XXX: All these filters are loaded on document creation. A document
// can, theoretically, contain zero or more images of different 
// depths. If there are zero images, then closing Krita will crash
// in the destructor.
class KisGaussianBlurFilter : public KisConvolutionConstFilter {
public:
	KisGaussianBlurFilter(KisView * view);
public:
	static inline QString name() { return i18n("Gaussian Blur"); };
};

class KisSharpenFilter : public KisConvolutionConstFilter {
public:
	KisSharpenFilter(KisView * view);
public:
	static inline QString name() { return i18n("Sharpen"); };
};

class KisMeanRemovalFilter : public KisConvolutionConstFilter {
public:
	KisMeanRemovalFilter(KisView * view);
public:
	static inline QString name() { return i18n("Mean Removal"); };
};

class KisEmbossLaplascianFilter : public KisConvolutionConstFilter {
public:
	KisEmbossLaplascianFilter(KisView * view);
public:
	static inline QString name() { return i18n("Emboss Laplascian"); };
};

class KisEmbossInAllDirectionsFilter : public KisConvolutionConstFilter {
public:
	KisEmbossInAllDirectionsFilter(KisView * view);
public:
	static inline QString name() { return i18n("Emboss in All Directions"); };
};

class KisEmbossHorizontalVerticalFilter : public KisConvolutionConstFilter {
public:
	KisEmbossHorizontalVerticalFilter(KisView * view);
public:
	static inline QString name() { return i18n("Emboss Horizontal & Vertical"); };
};

class KisEmbossVerticalFilter : public KisConvolutionConstFilter {
public:
	KisEmbossVerticalFilter(KisView * view);
public:
	static inline QString name() { return i18n("Emboss Vertical Only"); };
};

class KisEmbossHorizontalFilter : public KisConvolutionConstFilter {
public:
	KisEmbossHorizontalFilter(KisView * view);
public:
	static inline QString name() { return i18n("Emboss Horizontal Only"); };
};

class KisEmbossDiagonalFilter : public KisConvolutionConstFilter {
public:
	KisEmbossDiagonalFilter(KisView * view);
public:
	static inline QString name() { return i18n("Emboss Diagonal"); };
};

class KisTopEdgeDetectionFilter : public KisConvolutionConstFilter {
public:
	KisTopEdgeDetectionFilter(KisView * view);
public:
	static inline QString name() { return i18n("Top Edge Detections"); };
};

class KisRightEdgeDetectionFilter : public KisConvolutionConstFilter {
public:
	KisRightEdgeDetectionFilter(KisView * view);
public:
	static inline QString name() { return i18n("Right Edge Detections"); };
};

class KisBottomEdgeDetectionFilter : public KisConvolutionConstFilter {
public:
	KisBottomEdgeDetectionFilter(KisView * view);
public:
	static inline QString name() { return i18n("Bottom Edge Detections"); };
};

class KisLeftEdgeDetectionFilter : public KisConvolutionConstFilter {
public:
	KisLeftEdgeDetectionFilter(KisView * view);
public:
	static inline QString name() { return i18n("Left Edge Detections"); };
};


class KritaConvolutionFilters : public KParts::Plugin
{
public:
	KritaConvolutionFilters(QObject *parent, const char *name, const QStringList &);
	virtual ~KritaConvolutionFilters();
};

#endif
