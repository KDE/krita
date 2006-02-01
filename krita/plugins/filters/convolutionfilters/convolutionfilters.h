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

#ifndef CONVOLUTIONFILTERS_H
#define CONVOLUTIONFILTERS_H

#include <kparts/plugin.h>
#include "kis_convolution_filter.h"

class KisGaussianBlurFilter : public KisConvolutionConstFilter {
public:
    KisGaussianBlurFilter();
public:
    static inline KisID id() { return KisID("gaussian blur", i18n("Gaussian Blur")); };
    virtual bool supportsPainting() { return true; }
    virtual bool supportsIncrementalPainting() { return false; }
};

class KisSharpenFilter : public KisConvolutionConstFilter {
public:
    KisSharpenFilter();
public:
    static inline KisID id() { return KisID("sharpen", i18n("Sharpen")); };
    virtual bool supportsPainting() { return true; }
    virtual bool supportsIncrementalPainting() { return false; }
};

class KisMeanRemovalFilter : public KisConvolutionConstFilter {
public:
    KisMeanRemovalFilter();
public:
    static inline KisID id() { return KisID("mean removal", i18n("Mean Removal")); };
    virtual bool supportsPainting() { return false; }

};

class KisEmbossLaplascianFilter : public KisConvolutionConstFilter {
public:
    KisEmbossLaplascianFilter();
public:
    static inline KisID id() { return KisID("emboss laplascian", i18n("Emboss Laplascian")); };
    virtual bool supportsPainting() { return false; }

};

class KisEmbossInAllDirectionsFilter : public KisConvolutionConstFilter {
public:
    KisEmbossInAllDirectionsFilter();
public:
    static inline KisID id() { return KisID("emboss all directions", i18n("Emboss in All Directions")); };
    virtual bool supportsPainting() { return false; }

};

class KisEmbossHorizontalVerticalFilter : public KisConvolutionConstFilter {
public:
    KisEmbossHorizontalVerticalFilter();
public:
    static inline KisID id() { return KisID("", i18n("Emboss Horizontal & Vertical")); };
    virtual bool supportsPainting() { return false; }

};

class KisEmbossVerticalFilter : public KisConvolutionConstFilter {
public:
    KisEmbossVerticalFilter();
public:
    static inline KisID id() { return KisID("emboss vertical only", i18n("Emboss Vertical Only")); };
    virtual bool supportsPainting() { return false; }

};

class KisEmbossHorizontalFilter : public KisConvolutionConstFilter {
public:
    KisEmbossHorizontalFilter();
public:
    static inline KisID id() { return KisID("emboss horizontal only", i18n("Emboss Horizontal Only")); };
    virtual bool supportsPainting() { return false; }

};

class KisEmbossDiagonalFilter : public KisConvolutionConstFilter {
public:
    KisEmbossDiagonalFilter();
public:
    static inline KisID id() { return KisID("emboss diagonal", i18n("Emboss Diagonal")); };
    virtual bool supportsPainting() { return false; }

};

class KisTopEdgeDetectionFilter : public KisConvolutionConstFilter {
public:
    KisTopEdgeDetectionFilter();
public:
    static inline KisID id() { return KisID("top edge detections", i18n("Top Edge Detection")); };
    virtual bool supportsPainting() { return false; }

};

class KisRightEdgeDetectionFilter : public KisConvolutionConstFilter {
public:
    KisRightEdgeDetectionFilter();
public:
    static inline KisID id() { return KisID("right edge detections", i18n("Right Edge Detection")); };
    virtual bool supportsPainting() { return false; }

};

class KisBottomEdgeDetectionFilter : public KisConvolutionConstFilter {
public:
    KisBottomEdgeDetectionFilter();
public:
    static inline KisID id() { return KisID("bottom edge detections", i18n("Bottom Edge Detection")); };
    virtual bool supportsPainting() { return false; }

};

class KisLeftEdgeDetectionFilter : public KisConvolutionConstFilter {
public:
    KisLeftEdgeDetectionFilter();
public:
    static inline KisID id() { return KisID("left edge detections", i18n("Left Edge Detection")); };
    virtual bool supportsPainting() { return false; }

};


class KritaConvolutionFilters : public KParts::Plugin
{
public:
    KritaConvolutionFilters(QObject *parent, const char *name, const QStringList &);
    virtual ~KritaConvolutionFilters();
};

#endif
