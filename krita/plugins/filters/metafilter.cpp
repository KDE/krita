/*
 * Copyright (c) 2013 Boudewijn Rempt <boud@valdyas.org.
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

#include "metafilter.h"

#include <klocale.h>
#include <kcomponentdata.h>
#include <kstandarddirs.h>
#include <kpluginfactory.h>

#include "oilpaintfilter/kis_oilpaint_filter.h"
#include "roundcorners/kis_round_corners_filter.h"
#include "randompickfilter/randompickfilter.h"
#include "convolutionfilters/kis_convolution_filter.h"
#include "colorsfilters/kis_hsv_adjustment_filter.h"
#include "colorsfilters/kis_perchannel_filter.h"
#include "colorsfilters/kis_brightness_contrast_filter.h"
#include "raindropsfilter/kis_raindrops_filter.h"
#include "wavefilter/wavefilter.h"
#include "smalltilesfilter/kis_small_tiles_filter.h"
#include "sobelfilter/kis_sobel_filter.h"
#include "phongbumpmap/kis_phong_bumpmap_filter.h"
#include "embossfilter/kis_emboss_filter.h"
#include "noisefilter/noisefilter.h"
#include "metafilter.h"
#include "blur/kis_gaussian_blur_filter.h"
#include "blur/kis_motion_blur_filter.h"
#include "blur/kis_blur_filter.h"
#include "blur/kis_lens_blur_filter.h"
#include "levelfilter/kis_level_filter.h"
#include "levelfilter/levelfilter.h"
#include "pixelizefilter/kis_pixelize_filter.h"
#include "unsharp/kis_unsharp_filter.h"
#include "dodgeburn/DodgeBurn.h"
#include "example/example.h"
#include "fastcolortransfer/fastcolortransfer.h"
#include "imageenhancement/kis_simple_noise_reducer.h"
#include "imageenhancement/kis_wavelet_noise_reduction.h"
#include "colors/kis_minmax_filters.h"
#include "colors/kis_color_to_alpha.h"
#include "convolutionfilters/convolutionfilters.h"


K_PLUGIN_FACTORY(KritaMetaFilterFactory, registerPlugin<KritaMetaFilter();)
K_EXPORT_PLUGIN(KritaMetaFilterFactory("krita"))

KritaMetaFilter::KritaMetaFilter(QObject *parent, const QVariantList &)
        : QObject(parent)
{
    KisFilterRegistry::instance()->add(new KisBlurFilter());
    KisFilterRegistry::instance()->add(new KisGaussianBlurFilter());
    KisFilterRegistry::instance()->add(new KisMotionBlurFilter());
    KisFilterRegistry::instance()->add(new KisLensBlurFilter());
    KisFilterRegistry::instance()->add(new KisFilterDodgeBurn("dodge", "Dodge", i18n("Dodge")));
    KisFilterRegistry::instance()->add(new KisFilterDodgeBurn("burn", "Burn", i18n("Burn")));
    KisFilterRegistry::instance()->add(new KisEmbossFilter());
    KisFilterRegistry::instance()->add(new KisFilterInvert());
    KisFilterRegistry::instance()->add(new KisFilterFastColorTransfer());
    KisFilterRegistry::instance()->add(new KisSimpleNoiseReducer());
    KisFilterRegistry::instance()->add(new KisWaveletNoiseReduction());
    KisFilterRegistry::instance()->add(new KisLevelFilter());
    KisFilterRegistry::instance()->add(new KisFilterNoise());
    KisFilterRegistry::instance()->add(new KisOilPaintFilter());
    KisFilterRegistry::instance()->add(new KisFilterPhongBumpmap());
    KisFilterRegistry::instance()->add(new KisPixelizeFilter());
    KisFilterRegistry::instance()->add(new KisRainDropsFilter());
    KisFilterRegistry::instance()->add(new KisFilterRandomPick());
    KisFilterRegistry::instance()->add(new KisRoundCornersFilter());
    KisFilterRegistry::instance()->add(new KisSmallTilesFilter());
    KisFilterRegistry::instance()->add(new KisSobelFilter());
    KisFilterRegistry::instance()->add(new KisUnsharpFilter());
    KisFilterRegistry::instance()->add(new KisFilterWave());
    KisFilterRegistry::instance()->add(new KisFilterMax());
    KisFilterRegistry::instance()->add(new KisFilterMin());
    KisFilterRegistry::instance()->add(new KisFilterColorToAlpha());
    KisFilterRegistry::instance()->add(new KisSharpenFilter());
    KisFilterRegistry::instance()->add(new KisMeanRemovalFilter());
    KisFilterRegistry::instance()->add(new KisEmbossLaplascianFilter());
    KisFilterRegistry::instance()->add(new KisEmbossInAllDirectionsFilter());
    KisFilterRegistry::instance()->add(new KisEmbossHorizontalVerticalFilter());
    KisFilterRegistry::instance()->add(new KisEmbossVerticalFilter());
    KisFilterRegistry::instance()->add(new KisEmbossHorizontalFilter());
    KisFilterRegistry::instance()->add(new KisTopEdgeDetectionFilter());
    KisFilterRegistry::instance()->add(new KisRightEdgeDetectionFilter());
    KisFilterRegistry::instance()->add(new KisBottomEdgeDetectionFilter());
    KisFilterRegistry::instance()->add(new KisLeftEdgeDetectionFilter());
}

KritaMetaFilter::~KritaMetaFilter()
{
}

