/*
 * This file is part of the KDE project
 *
 * Copyright (c) 2016 Spencer Brown <sbrown655@gmail.com>
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

#include "krita_filter_gradient_map.h"
#include "KoColorSpace.h"
#include "kis_config_widget.h"
#include "filter/kis_color_transformation_filter.h"
#include "gradientmap.h"
#include "krita_gradient_map_color_transformation.h"


KritaFilterGradientMap::KritaFilterGradientMap() : KisColorTransformationFilter(id(), categoryMap(), i18n("&Gradient Map"))
{
    setShortcut(QKeySequence(Qt::SHIFT + Qt::ALT + Qt::CTRL + Qt::Key_I));
    setColorSpaceIndependence(TO_LAB16);
    setSupportsPainting(true);
    setShowConfigurationWidget(true);
}

KoColorTransformation* KritaFilterGradientMap::createTransformation(const KoColorSpace* cs, const KisFilterConfiguration* config) const
{
	auto gradient = static_cast<const KoAbstractGradient *>(dynamic_cast<const KritaGradientMapFilterConfiguration *>(config)->gradient());

	return new KritaGradientMapColorTransformation(gradient, cs);
}

KisConfigWidget * KritaFilterGradientMap::createConfigurationWidget(QWidget * parent, const KisPaintDeviceSP dev) const
{
	return new KritaGradientMapConfigWidget(parent, dev);
}

