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
#include <ksharedptr.h>

#include "kis_global.h"
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
typedef KisSharedPtrVector<KisImage> vKisImageSP;
typedef vKisImageSP::iterator vKisImageSP_it;
typedef vKisImageSP::const_iterator vKisImageSP_cit;

class KisPaintDeviceImpl;
typedef KSharedPtr<KisPaintDeviceImpl> KisPaintDeviceImplSP;
typedef KisSharedPtrVector<KisPaintDeviceImpl> vKisPaintDeviceImplSP;
typedef vKisPaintDeviceImplSP::iterator vKisPaintDeviceImplSP_it;
typedef vKisPaintDeviceImplSP::const_iterator vKisPaintDeviceImplSP_cit;

class KisLayer;
typedef KSharedPtr<KisLayer> KisLayerSP;
typedef KisSharedPtrVector<KisLayer> vKisLayerSP;
typedef vKisLayerSP::iterator vKisLayerSP_it;
typedef vKisLayerSP::const_iterator vKisLayerSP_cit;

class KisPartLayer;
typedef KSharedPtr<KisPartLayer> KisPartLayerSP;
typedef KisSharedPtrVector<KisPartLayer> vKisPartLayerSP;
typedef vKisPartLayerSP::iterator vKisPartLayerSP_it;
typedef vKisPartLayerSP::const_iterator vKisPartLayerSP_cit;

class KisSelection;
typedef KSharedPtr<KisSelection> KisSelectionSP;
typedef KisSharedPtrVector<KisSelection> vKisSelectionSP;
typedef vKisSelectionSP::iterator vKisSelectionSP_it;
typedef vKisSelectionSP::const_iterator vKisSelectionSP_cit;

class KisBackground;
typedef KSharedPtr<KisBackground> KisBackgroundSP;

class KisHistogram;
typedef KSharedPtr<KisHistogram> KisHistogramSP;
typedef KisSharedPtrVector<KisHistogram> vKisHistogramSP;
typedef vKisHistogramSP::iterator vKisHistogramSP_it;
typedef vKisHistogramSP::const_iterator vKisHistogramSP_cit;

class KisPaintOp;
typedef KSharedPtr<KisPaintOp> KisPaintOpSP;

class KisPaintOpFactory;
typedef KSharedPtr<KisPaintOpFactory> KisPaintOpFactorySP;

class KisToolFactory;
typedef KSharedPtr<KisToolFactory> KisToolFactorySP;

class KisPluginFactory;
typedef KSharedPtr<KisPluginFactory> KisPluginFactorySP;

class KisTool;
typedef KSharedPtr<KisTool> KisToolSP;
typedef KisSharedPtrVector<KisTool> vKisTool;
typedef vKisTool::iterator vKisTool_it;
typedef vKisTool::const_iterator vKisTool_cit;

class KisDataManager;
typedef KSharedPtr<KisDataManager> KisDataManagerSP;

typedef QValueVector<QPoint> vKisSegments;

class KisGuide;
typedef KSharedPtr<KisGuide> KisGuideSP;

class KisAlphaMask;
typedef KSharedPtr<KisAlphaMask> KisAlphaMaskSP;

class KisAlphaColorSpace;
typedef KSharedPtr<KisAlphaColorSpace> KisAlphaColorSpaceSP;


class KisFilter;
typedef KSharedPtr<KisFilter> KisFilterSP;


class KisAnnotation;
typedef KSharedPtr<KisAnnotation> KisAnnotationSP;
typedef KisSharedPtrVector<KisAnnotation> vKisAnnotationSP;
typedef vKisAnnotationSP::iterator vKisAnnotationSP_it;
typedef vKisAnnotationSP::const_iterator vKisAnnotationSP_cit;


class KisFileFormat;
typedef KSharedPtr<KisFileFormat> KisFileFormatSP;
typedef KisSharedPtrVector<KisFileFormat> vKisFileFormatSP;
typedef vKisFileFormatSP::iterator vKisFileFormatSP_it;
typedef vKisFileFormatSP::const_iterator vKisFileFormatSP_cit;

class KisMemento;
typedef KSharedPtr<KisMemento> KisMementoSP;

class KisColorSpace;
#endif // KISTYPES_H_
