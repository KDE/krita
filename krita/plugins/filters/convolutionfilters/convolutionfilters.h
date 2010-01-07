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

#include <QObject>
#include <QVariant>
#include "kis_convolution_filter.h"

class KisSharpenFilter : public KisConvolutionFilter
{
public:
    KisSharpenFilter();
public:
    static inline KoID id() {
        return KoID("sharpen", i18n("Sharpen"));
    }

};

class KisMeanRemovalFilter : public KisConvolutionFilter
{
public:
    KisMeanRemovalFilter();
public:
    static inline KoID id() {
        return KoID("mean removal", i18n("Mean Removal"));
    }
};

class KisEmbossLaplascianFilter : public KisConvolutionFilter
{
public:
    KisEmbossLaplascianFilter();
public:
    static inline KoID id() {
        return KoID("emboss laplascian", i18n("Emboss (Laplacian)"));
    }
};

class KisEmbossInAllDirectionsFilter : public KisConvolutionFilter
{
public:
    KisEmbossInAllDirectionsFilter();
public:
    static inline KoID id() {
        return KoID("emboss all directions", i18n("Emboss in All Directions"));
    }
};

class KisEmbossHorizontalVerticalFilter : public KisConvolutionFilter
{
public:
    KisEmbossHorizontalVerticalFilter();
public:
    static inline KoID id() {
        return KoID("emboss horizontal and vertical", i18n("Emboss Horizontal & Vertical"));
    }
};

class KisEmbossVerticalFilter : public KisConvolutionFilter
{
public:
    KisEmbossVerticalFilter();
public:
    static inline KoID id() {
        return KoID("emboss vertical only", i18n("Emboss Vertical Only"));
    }
};

class KisEmbossHorizontalFilter : public KisConvolutionFilter
{
public:
    KisEmbossHorizontalFilter();
public:
    static inline KoID id() {
        return KoID("emboss horizontal only", i18n("Emboss Horizontal Only"));
    }
};

class KisEmbossDiagonalFilter : public KisConvolutionFilter
{
public:
    KisEmbossDiagonalFilter();
public:
    static inline KoID id() {
        return KoID("emboss diagonal", i18n("Emboss Diagonal"));
    }
};

class KisTopEdgeDetectionFilter : public KisConvolutionFilter
{
public:
    KisTopEdgeDetectionFilter();
public:
    static inline KoID id() {
        return KoID("top edge detections", i18n("Top Edge Detection"));
    }
};

class KisRightEdgeDetectionFilter : public KisConvolutionFilter
{
public:
    KisRightEdgeDetectionFilter();
public:
    static inline KoID id() {
        return KoID("right edge detections", i18n("Right Edge Detection"));
    }
};

class KisBottomEdgeDetectionFilter : public KisConvolutionFilter
{
public:
    KisBottomEdgeDetectionFilter();
public:
    static inline KoID id() {
        return KoID("bottom edge detections", i18n("Bottom Edge Detection"));
    }
};

class KisLeftEdgeDetectionFilter : public KisConvolutionFilter
{
public:
    KisLeftEdgeDetectionFilter();
public:
    static inline KoID id() {
        return KoID("left edge detections", i18n("Left Edge Detection"));
    }
};


class KritaConvolutionFilters : public QObject
{
public:
    KritaConvolutionFilters(QObject *parent, const QVariantList &);
    virtual ~KritaConvolutionFilters();
};

#endif
