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

#include "convolutionfilters.h"

#include <stdlib.h>

#include <klocale.h>
#include <kcomponentdata.h>
#include <kgenericfactory.h>

#include <kis_convolution_kernel.h>
#include <kis_convolution_painter.h>

#include <filter/kis_filter_configuration.h>
#include <kis_selection.h>
#include <kis_paint_device.h>
#include <kis_processing_information.h>
#include <KoProgressUpdater.h>

#include <Eigen/Core>

using namespace Eigen;

typedef KGenericFactory<KritaConvolutionFilters> KritaConvolutionFiltersFactory;
K_EXPORT_COMPONENT_FACTORY(kritaconvolutionfilters, KritaConvolutionFiltersFactory("krita"))

KritaConvolutionFilters::KritaConvolutionFilters(QObject *parent, const QStringList &)
        : QObject(parent)
{
    //setComponentData(KritaConvolutionFiltersFactory::componentData());
    KisFilterRegistry * manager = KisFilterRegistry::instance();
    manager->add(new KisSharpenFilter());
    manager->add(new KisMeanRemovalFilter());
    manager->add(new KisEmbossLaplascianFilter());
    manager->add(new KisEmbossInAllDirectionsFilter());
    manager->add(new KisEmbossHorizontalVerticalFilter());
    manager->add(new KisEmbossVerticalFilter());
    manager->add(new KisEmbossHorizontalFilter());
    manager->add(new KisTopEdgeDetectionFilter());
    manager->add(new KisRightEdgeDetectionFilter());
    manager->add(new KisBottomEdgeDetectionFilter());
    manager->add(new KisLeftEdgeDetectionFilter());

}

KritaConvolutionFilters::~KritaConvolutionFilters()
{
}

KisSharpenFilter::KisSharpenFilter()
        : KisConvolutionFilter(id(), categoryEnhance(), i18n("&Sharpen"))
{
    setSupportsPainting(true);
    setSupportsIncrementalPainting(false);

    Matrix<qreal, Dynamic, Dynamic> kernelMatrix(3, 3);
    kernelMatrix <<  0, -2,   0,
                    -2,  11, -2,
                     0, -2,   0;

    m_matrix = KisConvolutionKernel::fromMatrix(kernelMatrix, 0, 3);
}

KisMeanRemovalFilter::KisMeanRemovalFilter()
        : KisConvolutionFilter(id(), categoryEnhance(), i18n("&Mean Removal"))
{
    setSupportsPainting(false);

    Matrix<qreal, Dynamic, Dynamic> kernelMatrix(3, 3);
    kernelMatrix << -1, -1, -1,
                    -1,  9, -1,
                    -1, -1, -1;

    m_matrix = KisConvolutionKernel::fromMatrix(kernelMatrix, 0, 1);
}

KisEmbossLaplascianFilter::KisEmbossLaplascianFilter()
        : KisConvolutionFilter(id(), categoryEmboss(), i18n("Emboss (Laplacian)"))
{
    setSupportsPainting(false);

    Matrix<qreal, Dynamic, Dynamic> kernelMatrix(3, 3);
    kernelMatrix << -1, 0, -1,
                     0, 4,  0,
                    -1, 0, -1;

    m_matrix = KisConvolutionKernel::fromMatrix(kernelMatrix, 127, 1);
}

KisEmbossInAllDirectionsFilter::KisEmbossInAllDirectionsFilter()
        : KisConvolutionFilter(id(), categoryEmboss(), i18n("Emboss in All Directions"))
{
    setSupportsPainting(false);

    Matrix<qreal, Dynamic, Dynamic> kernelMatrix(3, 3);
    kernelMatrix << -1, -1, -1,
                    -1,  8, -1,
                    -1, -1, -1;

    m_matrix = KisConvolutionKernel::fromMatrix(kernelMatrix, 127, 1);
}

KisEmbossHorizontalVerticalFilter::KisEmbossHorizontalVerticalFilter()
        : KisConvolutionFilter(id(), categoryEmboss(), i18n("Emboss Horizontal && Vertical"))
{
    setSupportsPainting(false);

    Matrix<qreal, Dynamic, Dynamic> kernelMatrix(3, 3);
    kernelMatrix <<  0, -1,  0,
                    -1,  4, -1,
                     0, -1,  0;

    m_matrix = KisConvolutionKernel::fromMatrix(kernelMatrix, 127, 1);
}

KisEmbossVerticalFilter::KisEmbossVerticalFilter()
        : KisConvolutionFilter(id(), categoryEmboss(), i18n("Emboss Vertical Only"))
{
    setSupportsPainting(false);

    Matrix<qreal, Dynamic, Dynamic> kernelMatrix(3, 3);
    kernelMatrix << 0, -1, 0,
                    0,  2, 0,
                    0, -1, 0;

    m_matrix = KisConvolutionKernel::fromMatrix(kernelMatrix, 127, 1);
}

KisEmbossHorizontalFilter::KisEmbossHorizontalFilter() :
        KisConvolutionFilter(id(), categoryEmboss(), i18n("Emboss Horizontal Only"))
{
    setSupportsPainting(false);

    Matrix<qreal, Dynamic, Dynamic> kernelMatrix(3, 3);
    kernelMatrix <<  0, 0,  0,
                    -1, 2, -1,
                     0, 0,  0;

    m_matrix = KisConvolutionKernel::fromMatrix(kernelMatrix, 127, 1);
}

KisEmbossDiagonalFilter::KisEmbossDiagonalFilter()
        : KisConvolutionFilter(id(), categoryEdgeDetection(), i18n("Top Edge Detection"))
{
    setSupportsPainting(false);

    Matrix<qreal, Dynamic, Dynamic> kernelMatrix(3, 3);
    kernelMatrix << -1, 0, -1,
                     0, 4,  0,
                    -1, 0, -1;

    m_matrix = KisConvolutionKernel::fromMatrix(kernelMatrix, 127, 1);
}


KisTopEdgeDetectionFilter::KisTopEdgeDetectionFilter()
        : KisConvolutionFilter(id(), categoryEdgeDetection(), i18n("Top Edge Detection"))
{
    setSupportsPainting(false);

    Matrix<qreal, Dynamic, Dynamic> kernelMatrix(3, 3);
    kernelMatrix <<  1,  1,  1,
                     0,  0,  0,
                    -1, -1, -1;

    m_matrix = KisConvolutionKernel::fromMatrix(kernelMatrix, 127, 1);
}

KisRightEdgeDetectionFilter::KisRightEdgeDetectionFilter()
        : KisConvolutionFilter(id(), categoryEdgeDetection(), i18n("Right Edge Detection"))
{
    setSupportsPainting(false);

    Matrix<qreal, Dynamic, Dynamic> kernelMatrix(3, 3);
    kernelMatrix << -1, 0, 1,
                    -1, 0, 1,
                    -1, 0, 1;

    m_matrix = KisConvolutionKernel::fromMatrix(kernelMatrix, 127, 1);
}

KisBottomEdgeDetectionFilter::KisBottomEdgeDetectionFilter() : KisConvolutionFilter(id(), categoryEdgeDetection(), i18n("Bottom Edge Detection"))
{
    setSupportsPainting(false);

    Matrix<qreal, Dynamic, Dynamic> kernelMatrix(3, 3);
    kernelMatrix << -1, -1, -1,
                     0,  0,  0,
                     1,  1,  1;

    m_matrix = KisConvolutionKernel::fromMatrix(kernelMatrix, 127, 1);
}

KisLeftEdgeDetectionFilter::KisLeftEdgeDetectionFilter() : KisConvolutionFilter(id(), categoryEdgeDetection(), i18n("Left Edge Detection"))
{
    setSupportsPainting(false);

    Matrix<qreal, Dynamic, Dynamic> kernelMatrix(3, 3);
    kernelMatrix << 1, 0, -1,
                    1, 0, -1,
                    1, 0, -1;

    m_matrix = KisConvolutionKernel::fromMatrix(kernelMatrix, 127, 1);
}
