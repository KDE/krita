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
        virtual bool supportsPainting() { return true; }

};

class KisSharpenFilter : public KisConvolutionConstFilter {
public:
	KisSharpenFilter(KisView * view);
public:
	static inline QString name() { return i18n("Sharpen"); };
        virtual bool supportsPainting() { return true; }

};

class KisMeanRemovalFilter : public KisConvolutionConstFilter {
public:
	KisMeanRemovalFilter(KisView * view);
public:
	static inline QString name() { return i18n("Mean Removal"); };
        virtual bool supportsPainting() { return true; }

};

class KisEmbossLaplascianFilter : public KisConvolutionConstFilter {
public:
	KisEmbossLaplascianFilter(KisView * view);
public:
	static inline QString name() { return i18n("Emboss Laplascian"); };
        virtual bool supportsPainting() { return true; }

};

class KisEmbossInAllDirectionsFilter : public KisConvolutionConstFilter {
public:
	KisEmbossInAllDirectionsFilter(KisView * view);
public:
	static inline QString name() { return i18n("Emboss in All Directions"); };
        virtual bool supportsPainting() { return true; }

};

class KisEmbossHorizontalVerticalFilter : public KisConvolutionConstFilter {
public:
	KisEmbossHorizontalVerticalFilter(KisView * view);
public:
	static inline QString name() { return i18n("Emboss Horizontal & Vertical"); };
        virtual bool supportsPainting() { return true; }

};

class KisEmbossVerticalFilter : public KisConvolutionConstFilter {
public:
	KisEmbossVerticalFilter(KisView * view);
public:
	static inline QString name() { return i18n("Emboss Vertical Only"); };
        virtual bool supportsPainting() { return true; }

};

class KisEmbossHorizontalFilter : public KisConvolutionConstFilter {
public:
	KisEmbossHorizontalFilter(KisView * view);
public:
	static inline QString name() { return i18n("Emboss Horizontal Only"); };
        virtual bool supportsPainting() { return true; }

};

class KisEmbossDiagonalFilter : public KisConvolutionConstFilter {
public:
	KisEmbossDiagonalFilter(KisView * view);
public:
	static inline QString name() { return i18n("Emboss Diagonal"); };
        virtual bool supportsPainting() { return true; }

};

class KisTopEdgeDetectionFilter : public KisConvolutionConstFilter {
public:
	KisTopEdgeDetectionFilter(KisView * view);
public:
	static inline QString name() { return i18n("Top Edge Detections"); };
        virtual bool supportsPainting() { return true; }

};

class KisRightEdgeDetectionFilter : public KisConvolutionConstFilter {
public:
	KisRightEdgeDetectionFilter(KisView * view);
public:
	static inline QString name() { return i18n("Right Edge Detections"); };
        virtual bool supportsPainting() { return true; }

};

class KisBottomEdgeDetectionFilter : public KisConvolutionConstFilter {
public:
	KisBottomEdgeDetectionFilter(KisView * view);
public:
	static inline QString name() { return i18n("Bottom Edge Detections"); };
        virtual bool supportsPainting() { return true; }

};

class KisLeftEdgeDetectionFilter : public KisConvolutionConstFilter {
public:
	KisLeftEdgeDetectionFilter(KisView * view);
public:
	static inline QString name() { return i18n("Left Edge Detections"); };
        virtual bool supportsPainting() { return true; }

};


class KritaConvolutionFilters : public KParts::Plugin
{
public:
	KritaConvolutionFilters(QObject *parent, const char *name, const QStringList &);
	virtual ~KritaConvolutionFilters();
};

#endif
