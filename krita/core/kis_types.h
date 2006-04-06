/*
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
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
#ifndef KISTYPES_H_
#define KISTYPES_H_

#include <qvaluevector.h>
#include <qmap.h>
#include <qpoint.h>

#include <ksharedptr.h>

#include "kis_shared_ptr_vector.h"

/**
 * Define lots of shared pointer versions of Krita classes.
 * Shared pointer classes have the advantage of near automatic
 * memory management (but take care of circular references)
 * and the disadvantage that inheritiance relations are no longer
 * recognizable
 */

class KisImage;
typedef KSharedPtr<KisImage> KisImageSP;

class KisPaintDevice;
typedef KSharedPtr<KisPaintDevice> KisPaintDeviceSP;
typedef KisSharedPtrVector<KisPaintDevice> vKisPaintDeviceSP;
typedef vKisPaintDeviceSP::iterator vKisPaintDeviceSP_it;
typedef vKisPaintDeviceSP::const_iterator vKisPaintDeviceSP_cit;

class KisLayer;
typedef KSharedPtr<KisLayer> KisLayerSP;
typedef KisSharedPtrVector<KisLayer> vKisLayerSP;
typedef vKisLayerSP::iterator vKisLayerSP_it;
typedef vKisLayerSP::const_iterator vKisLayerSP_cit;

class KisPartLayer;
typedef KSharedPtr<KisPartLayer> KisPartLayerSP;

class KisPaintLayer;
typedef KSharedPtr<KisPaintLayer> KisPaintLayerSP;

class KisAdjustmentLayer;
typedef KSharedPtr<KisAdjustmentLayer> KisAdjustmentLayerSP;

class KisGroupLayer;
typedef KSharedPtr<KisGroupLayer> KisGroupLayerSP;

class KisSelection;
typedef KSharedPtr<KisSelection> KisSelectionSP;

class KisBackground;
typedef KSharedPtr<KisBackground> KisBackgroundSP;

class KisSubstrate;
typedef KSharedPtr<KisSubstrate> KisSubstrateSP;

class KisHistogram;
typedef KSharedPtr<KisHistogram> KisHistogramSP;

class KisPaintOpFactory;
typedef KSharedPtr<KisPaintOpFactory> KisPaintOpFactorySP;

typedef QValueVector<QPoint> vKisSegments;

//class KisGuide;
//typedef KSharedPtr<KisGuide> KisGuideSP;

class KisAlphaMask;
typedef KSharedPtr<KisAlphaMask> KisAlphaMaskSP;

class KisFilter;
typedef KSharedPtr<KisFilter> KisFilterSP;

#endif // KISTYPES_H_
