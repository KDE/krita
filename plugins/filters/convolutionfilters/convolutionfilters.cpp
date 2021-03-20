/*
 * This file is part of the KDE project
 *
 * SPDX-FileCopyrightText: 2004 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "convolutionfilters.h"

#include <stdlib.h>

#include <klocalizedstring.h>

#include <kpluginfactory.h>

#include <kis_convolution_kernel.h>
#include <kis_convolution_painter.h>

#include <filter/kis_filter_category_ids.h>
#include <filter/kis_filter_configuration.h>
#include <kis_selection.h>
#include <kis_paint_device.h>
#include <kis_processing_information.h>

#include <Eigen/Core>

K_PLUGIN_FACTORY_WITH_JSON(KritaConvolutionFiltersFactory, "kritaconvolutionfilters.json", registerPlugin<KritaConvolutionFilters>();)

KritaConvolutionFilters::KritaConvolutionFilters(QObject *parent, const QVariantList &)
        : QObject(parent)
{
    KisFilterRegistry * manager = KisFilterRegistry::instance();
    manager->add(new KisSharpenFilter());
    manager->add(new KisMeanRemovalFilter());
    manager->add(new KisEmbossLaplascianFilter());
    manager->add(new KisEmbossInAllDirectionsFilter());
    manager->add(new KisEmbossHorizontalVerticalFilter());
    manager->add(new KisEmbossVerticalFilter());
    manager->add(new KisEmbossHorizontalFilter());
}

KritaConvolutionFilters::~KritaConvolutionFilters()
{
}

KisSharpenFilter::KisSharpenFilter()
        : KisConvolutionFilter(id(), FiltersCategoryEnhanceId, i18n("&Sharpen"))
{
    setSupportsPainting(true);
    setShowConfigurationWidget(false);

    Eigen::Matrix<qreal, Eigen::Dynamic, Eigen::Dynamic> kernelMatrix(3, 3);
    kernelMatrix <<  0, -2,   0,
                    -2,  11, -2,
                     0, -2,   0;

    m_matrix = KisConvolutionKernel::fromMatrix(kernelMatrix, 0, 3);
}

KisMeanRemovalFilter::KisMeanRemovalFilter()
        : KisConvolutionFilter(id(), FiltersCategoryEnhanceId, i18n("&Mean Removal"))
{
    setSupportsPainting(false);
    setShowConfigurationWidget(false);

    Eigen::Matrix<qreal, Eigen::Dynamic, Eigen::Dynamic> kernelMatrix(3, 3);
    kernelMatrix << -1, -1, -1,
                    -1,  9, -1,
                    -1, -1, -1;

    m_matrix = KisConvolutionKernel::fromMatrix(kernelMatrix, 0, 1);
}

KisEmbossLaplascianFilter::KisEmbossLaplascianFilter()
        : KisConvolutionFilter(id(), FiltersCategoryEmbossId, i18n("Emboss (Laplacian)"))
{
    setSupportsPainting(false);
    setShowConfigurationWidget(false);

    Eigen::Matrix<qreal, Eigen::Dynamic, Eigen::Dynamic> kernelMatrix(3, 3);
    kernelMatrix << -1, 0, -1,
                     0, 4,  0,
                    -1, 0, -1;

    m_matrix = KisConvolutionKernel::fromMatrix(kernelMatrix, 0.5, 1);
    setIgnoreAlpha(true);
}

KisEmbossInAllDirectionsFilter::KisEmbossInAllDirectionsFilter()
        : KisConvolutionFilter(id(), FiltersCategoryEmbossId, i18n("Emboss in All Directions"))
{
    setSupportsPainting(false);
    setShowConfigurationWidget(false);

    Eigen::Matrix<qreal, Eigen::Dynamic, Eigen::Dynamic> kernelMatrix(3, 3);
    kernelMatrix << -1, -1, -1,
                    -1,  8, -1,
                    -1, -1, -1;

    m_matrix = KisConvolutionKernel::fromMatrix(kernelMatrix, 0.5, 1);
    setIgnoreAlpha(true);
}

KisEmbossHorizontalVerticalFilter::KisEmbossHorizontalVerticalFilter()
        : KisConvolutionFilter(id(), FiltersCategoryEmbossId, i18n("Emboss Horizontal && Vertical"))
{
    setSupportsPainting(false);
    setShowConfigurationWidget(false);

    Eigen::Matrix<qreal, Eigen::Dynamic, Eigen::Dynamic> kernelMatrix(3, 3);
    kernelMatrix <<  0, -1,  0,
                    -1,  4, -1,
                     0, -1,  0;

    m_matrix = KisConvolutionKernel::fromMatrix(kernelMatrix, 0.5, 1);
    setIgnoreAlpha(true);
}

KisEmbossVerticalFilter::KisEmbossVerticalFilter()
        : KisConvolutionFilter(id(), FiltersCategoryEmbossId, i18n("Emboss Vertical Only"))
{
    setSupportsPainting(false);
    setShowConfigurationWidget(false);

    Eigen::Matrix<qreal, Eigen::Dynamic, Eigen::Dynamic> kernelMatrix(3, 3);
    kernelMatrix << 0, -1, 0,
                    0,  2, 0,
                    0, -1, 0;

    m_matrix = KisConvolutionKernel::fromMatrix(kernelMatrix, 0.5, 1);
    setIgnoreAlpha(true);
}

KisEmbossHorizontalFilter::KisEmbossHorizontalFilter() :
        KisConvolutionFilter(id(), FiltersCategoryEmbossId, i18n("Emboss Horizontal Only"))
{
    setSupportsPainting(false);
    setShowConfigurationWidget(false);

    Eigen::Matrix<qreal, Eigen::Dynamic, Eigen::Dynamic> kernelMatrix(3, 3);
    kernelMatrix <<  0, 0,  0,
                    -1, 2, -1,
                     0, 0,  0;

    m_matrix = KisConvolutionKernel::fromMatrix(kernelMatrix, 0.5, 1);
    setIgnoreAlpha(true);
}

KisEmbossDiagonalFilter::KisEmbossDiagonalFilter()
        : KisConvolutionFilter(id(), FiltersCategoryEdgeDetectionId, i18n("Top Edge Detection"))
{
    setSupportsPainting(false);
    setShowConfigurationWidget(false);

    Eigen::Matrix<qreal, Eigen::Dynamic, Eigen::Dynamic> kernelMatrix(3, 3);
    kernelMatrix << -1, 0, -1,
                     0, 4,  0,
                    -1, 0, -1;

    m_matrix = KisConvolutionKernel::fromMatrix(kernelMatrix, 0.5, 1);
    setIgnoreAlpha(true);
}

#include "convolutionfilters.moc"
