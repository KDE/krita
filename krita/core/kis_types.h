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
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */
#if !defined KISTYPES_H_
#define KISTYPES_H_

#include <qvaluevector.h>
#include <qmap.h>
#include <ksharedptr.h>
#include <koColor.h>

#include "kis_global.h"
#include "kis_shared_ptr_vector.h"

class KisImage;
typedef KSharedPtr<KisImage> KisImageSP;
typedef KisSharedPtrVector<KisImage> vKisImageSP;
typedef vKisImageSP::iterator vKisImageSP_it;
typedef vKisImageSP::const_iterator vKisImageSP_cit;

class KisPaintDevice;
typedef KSharedPtr<KisPaintDevice> KisPaintDeviceSP;
typedef KisSharedPtrVector<KisPaintDevice> vKisPaintDeviceSP;
typedef vKisPaintDeviceSP::iterator vKisPaintDeviceSP_it;
typedef vKisPaintDeviceSP::const_iterator vKisPaintDeviceSP_cit;

class KisChannel;
typedef KSharedPtr<KisChannel> KisChannelSP;
typedef KisSharedPtrVector<KisChannel> vKisChannelSP;
typedef vKisChannelSP::iterator vKisChannelSP_it;
typedef vKisChannelSP::const_iterator vKisChannelSP_cit;

class KisMask;
typedef KSharedPtr<KisMask> KisMaskSP;
typedef KisSharedPtrVector<KisMask> vKisMaskSP;
typedef vKisMaskSP::iterator vKisMaskSP_it;
typedef vKisMaskSP::const_iterator vKisMaskSP_cit;

class KisLayer;
typedef KSharedPtr<KisLayer> KisLayerSP;
typedef KisSharedPtrVector<KisLayer> vKisLayerSP;
typedef vKisLayerSP::iterator vKisLayerSP_it;
typedef vKisLayerSP::const_iterator vKisLayerSP_cit;

class KisSelection;
typedef KSharedPtr<KisSelection> KisSelectionSP;
typedef KisSharedPtrVector<KisSelection> vKisSelectionSP;
typedef vKisSelectionSP::iterator vKisSelectionSP_it;
typedef vKisSelectionSP::const_iterator vKisSelectionSP_cit;

class KisBackground;
typedef KSharedPtr<KisBackground> KisBackgroundSP;

class KisFloatingSelection;
typedef KSharedPtr<KisFloatingSelection> KisFloatingSelectionSP;

class KisTile;
typedef KSharedPtr<KisTile> KisTileSP;
typedef KisSharedPtrVector<KisTile> vKisTileSP;
typedef vKisTileSP::iterator vKisTileSP_it;
typedef vKisTileSP::const_iterator vKisTileSP_cit;

class KisHistogram;
typedef KSharedPtr<KisHistogram> KisHistogramSP;
typedef KisSharedPtrVector<KisHistogram> vKisHistogramSP;
typedef vKisHistogramSP::iterator vKisHistogramSP_it;
typedef vKisHistogramSP::const_iterator vKisHistogramSP_cit;

class KisPaintOpMaker;
typedef KSharedPtr<KisPaintOpMaker> KisPaintOpMakerSP;

class KisPluginRegistry;
typedef KSharedPtr<KisPluginRegistry> KisPluginRegistrySP;

class KisTool;
typedef KSharedPtr<KisTool> KisToolSP;
typedef KisSharedPtrVector<KisTool> vKisTool;
typedef vKisTool::iterator vKisTool_it;
typedef vKisTool::const_iterator vKisTool_cit;

class KisTileMgr;
typedef KSharedPtr<KisTileMgr> KisTileMgrSP;

class KisPixelData;
typedef KSharedPtr<KisPixelData> KisPixelDataSP;

typedef QValueVector<QPoint> vKisSegments;

class KoColor;
typedef QValueVector<KoColor> KoColorMap;

class KisStrategyColorSpace;
typedef KSharedPtr<KisStrategyColorSpace> KisStrategyColorSpaceSP;
typedef QMap<Q_INT32, KisStrategyColorSpaceSP> KisStrategyColorSpaceMap;

class KisGuide;
typedef KSharedPtr<KisGuide> KisGuideSP;

class KisAlphaMask;
typedef KSharedPtr<KisAlphaMask> KisAlphaMaskSP;

class KisFilter;
typedef KSharedPtr<KisFilter> KisFilterSP;

class KisFilterRegistry;
typedef KSharedPtr<KisFilterRegistry> KisFilterRegistrySP;

#endif // KISTYPES_H_
