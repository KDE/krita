/*
 * This file is part of Krita
 *
 * SPDX-FileCopyrightText: 2004 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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

class KritaConvolutionFilters : public QObject
{
    Q_OBJECT
public:
    KritaConvolutionFilters(QObject *parent, const QVariantList &);
    ~KritaConvolutionFilters() override;
};

#endif
