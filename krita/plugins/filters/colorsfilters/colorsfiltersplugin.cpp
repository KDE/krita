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
#include "colorsfiltersplugin.h"
#include "colorsfilters.h"

#include <math.h>

#include <stdlib.h>
#include <string.h>

#include <QSlider>
#include <QPoint>
#include <QColor>

#include <klocale.h>
#include <kcomponentdata.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <kis_debug.h>
#include <kpluginfactory.h>

#include "KoBasicHistogramProducers.h"
#include <KoColorSpace.h>
#include <KoColorTransformation.h>
#include <filter/kis_filter_configuration.h>
#include <kis_paint_device.h>
#include <kis_processing_information.h>
#include <kis_doc2.h>
#include <kis_image.h>
#include <kis_layer.h>
#include <kis_global.h>
#include <kis_types.h>
#include <kis_selection.h>
#include "kis_histogram.h"
#include "kis_hsv_adjustment_filter.h"
#include "kis_brightness_contrast_filter.h"
#include "kis_perchannel_filter.h"
#include "filter/kis_filter_registry.h"
#include <kis_painter.h>
#include <KoProgressUpdater.h>
#include <KoUpdater.h>
#include <KoColorSpaceConstants.h>
#include <KoCompositeOp.h>
#include <kis_iterator_ng.h>

K_PLUGIN_FACTORY(ColorsFiltersFactory, registerPlugin<ColorsFilters>();)
K_EXPORT_PLUGIN(ColorsFiltersFactory("krita"))

ColorsFilters::ColorsFilters(QObject *parent, const QVariantList &)
        : QObject(parent)
{
    KisFilterRegistry * manager = KisFilterRegistry::instance();
    manager->add(new KisBrightnessContrastFilter());
    manager->add(new KisAutoContrast());
    manager->add(new KisPerChannelFilter());
    manager->add(new KisDesaturateFilter());
    manager->add(new KisHSVAdjustmentFilter());

}

ColorsFilters::~ColorsFilters()
{
}
