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
#include "convolutionfiltersplugin.h"
#include "convolutionfilters.h"

#include <stdlib.h>

#include <klocale.h>
#include <kcomponentdata.h>
#include <kpluginfactory.h>

#include <kis_convolution_kernel.h>
#include <kis_convolution_painter.h>

#include <filter/kis_filter_configuration.h>
#include <kis_selection.h>
#include <kis_paint_device.h>
#include <kis_processing_information.h>
#include <KoProgressUpdater.h>

#include <Eigen/Core>

using namespace Eigen;

K_PLUGIN_FACTORY(KritaConvolutionFiltersFactory, registerPlugin<KritaConvolutionFilters>();)
K_EXPORT_PLUGIN(KritaConvolutionFiltersFactory("krita"))

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
    manager->add(new KisTopEdgeDetectionFilter());
    manager->add(new KisRightEdgeDetectionFilter());
    manager->add(new KisBottomEdgeDetectionFilter());
    manager->add(new KisLeftEdgeDetectionFilter());

}

KritaConvolutionFilters::~KritaConvolutionFilters()
{
}

