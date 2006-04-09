/*
 * This file is part of the KDE project
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

#include <stdlib.h>

#include <klocale.h>
#include <kinstance.h>
#include <kgenericfactory.h>

#include <kis_convolution_painter.h>
#include "convolutionfilters.h"

#include "kis_custom_convolution_filter.h"

KisKernelSP createKernel( qint32 i0, qint32 i1, qint32 i2,
                          qint32 i3, qint32 i4, qint32 i5,
                          qint32 i6, qint32 i7, qint32 i8,
                          qint32 factor, qint32 offset )
{
    KisKernelSP kernel = KisKernelSP(new KisKernel());
    kernel->width = 3;
    kernel->height = 3;

    kernel->factor = factor;
    kernel->offset = offset;

    kernel->data = new qint32[9];
    kernel->data[0] = i0;
    kernel->data[1] = i1;
    kernel->data[2] = i2;
    kernel->data[3] = i3;
    kernel->data[4] = i4;
    kernel->data[5] = i5;
    kernel->data[6] = i6;
    kernel->data[7] = i7;
    kernel->data[8] = i8;

    return kernel;
}



typedef KGenericFactory<KritaConvolutionFilters> KritaConvolutionFiltersFactory;
K_EXPORT_COMPONENT_FACTORY( kritaconvolutionfilters, KritaConvolutionFiltersFactory( "krita" ) )

KritaConvolutionFilters::KritaConvolutionFilters(QObject *parent, const char *name, const QStringList &)
        : KParts::Plugin(parent)
{
    setObjectName(name);
    setInstance(KritaConvolutionFiltersFactory::instance());

    if (parent->inherits("KisFilterRegistry")) {
        KisFilterRegistry * manager = dynamic_cast<KisFilterRegistry *>(parent);
        manager->add(KisFilterSP(new KisGaussianBlurFilter()));
        manager->add(KisFilterSP(new KisSharpenFilter()));
        manager->add(KisFilterSP(new KisMeanRemovalFilter()));
        manager->add(KisFilterSP(new KisEmbossLaplascianFilter()));
        manager->add(KisFilterSP(new KisEmbossInAllDirectionsFilter()));
        manager->add(KisFilterSP(new KisEmbossHorizontalVerticalFilter()));
        manager->add(KisFilterSP(new KisEmbossVerticalFilter()));
        manager->add(KisFilterSP(new KisEmbossHorizontalFilter()));
        manager->add(KisFilterSP(new KisTopEdgeDetectionFilter()));
        manager->add(KisFilterSP(new KisRightEdgeDetectionFilter()));
        manager->add(KisFilterSP(new KisBottomEdgeDetectionFilter()));
        manager->add(KisFilterSP(new KisLeftEdgeDetectionFilter()));
        manager->add(KisFilterSP(new KisCustomConvolutionFilter()));
    }
}

KritaConvolutionFilters::~KritaConvolutionFilters()
{
}

KisGaussianBlurFilter::KisGaussianBlurFilter()
    : KisConvolutionConstFilter(id(), "blur", i18n("&Gaussian Blur"))
{
    m_matrix = createKernel( 1, 2, 1, 2, 4, 2, 1, 2, 1, 16, 0);
}


KisSharpenFilter::KisSharpenFilter()
    : KisConvolutionConstFilter(id(), "enhance", i18n("&Sharpen"))
{
    m_matrix = createKernel( 0, -2, 0, -2, 11, -2, 0, -2, 0, 3, 0);
}

KisMeanRemovalFilter::KisMeanRemovalFilter()
    : KisConvolutionConstFilter(id(), "enhance", i18n("&Mean Removal"))
{
    m_matrix = createKernel( -1, -1, -1, -1, 9, -1, -1, -1, -1, 1, 0);
}

KisEmbossLaplascianFilter::KisEmbossLaplascianFilter()
    : KisConvolutionConstFilter(id(), "emboss", i18n("Emboss Laplascian"))
{
    m_matrix = createKernel( -1, 0, -1 , 0, 4, 0 , -1, 0, -1, 1, 127);
}

KisEmbossInAllDirectionsFilter::KisEmbossInAllDirectionsFilter()
    : KisConvolutionConstFilter(id(), "emboss", i18n("Emboss in All Directions"))
{
    m_matrix = createKernel( -1, -1, -1 , -1, 8, -1 , -1, -1, -1, 1, 127);
}

KisEmbossHorizontalVerticalFilter::KisEmbossHorizontalVerticalFilter()
    : KisConvolutionConstFilter(id(), "emboss", i18n("Emboss Horizontal && Vertical"))
{
    m_matrix = createKernel( 0, -1, 0 , -1, 4, -1 , 0, -1, 0, 1, 127);
}

KisEmbossVerticalFilter::KisEmbossVerticalFilter()
    : KisConvolutionConstFilter(id(), "emboss", i18n("Emboss Vertical Only"))
{
    m_matrix = createKernel( 0, -1, 0 , 0, 2, 0 , 0, -1, 0, 1, 127);
}

KisEmbossHorizontalFilter::KisEmbossHorizontalFilter() :
    KisConvolutionConstFilter(id(), "emboss", i18n("Emboss Horizontal Only"))
{
    m_matrix = createKernel( 0, 0, 0 , -1, 4, -1 , 0, 0, 0, 1, 127);

}

KisEmbossDiagonalFilter::KisEmbossDiagonalFilter()
    : KisConvolutionConstFilter(id(), "edge", i18n("Top Edge Detection"))
{
    m_matrix = createKernel( -1, 0, -1 , 0, 4, 0 , -1, 0, -1, 1, 127);
}


KisTopEdgeDetectionFilter::KisTopEdgeDetectionFilter()
    : KisConvolutionConstFilter(id(), "edge", i18n("Top Edge Detection"))
{
    m_matrix = createKernel( 1, 1, 1 , 0, 0, 0 , -1, -1, -1, 1, 127);

}

KisRightEdgeDetectionFilter::KisRightEdgeDetectionFilter()
    : KisConvolutionConstFilter(id(), "edge", i18n("Right Edge Detection"))
{
    m_matrix = createKernel(  -1, 0, 1 , -1, 0, 1 , -1, 0, 1,  1, 127);
}

KisBottomEdgeDetectionFilter::KisBottomEdgeDetectionFilter() : KisConvolutionConstFilter(id(), "edge", i18n("Bottom Edge Detection"))
{
    m_matrix = createKernel( -1, -1, -1 , 0, 0, 0 , 1, 1, 1, 1, 127);
}

KisLeftEdgeDetectionFilter::KisLeftEdgeDetectionFilter() : KisConvolutionConstFilter(id(), "edge", i18n("Left Edge Detection"))
{
    m_matrix = createKernel( 1, 0, -1 , 1, 0, -1 , 1, 0, -1, 1, 127);
}
