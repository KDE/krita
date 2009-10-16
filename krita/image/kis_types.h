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

#include <QVector>
#include <QPoint>

template<class T>
class KisWeakSharedPtr;
template<class T>
class KisSharedPtr;

#include "kis_shared_ptr_vector.h"

/**
 * Define lots of shared pointer versions of Krita classes.
 * Shared pointer classes have the advantage of near automatic
 * memory management (but beware of circular references)
 * and the disadvantage that inheritiance relations are no longer
 * recognizable
 */
class KisImage;
typedef KisSharedPtr<KisImage> KisImageSP;
typedef KisWeakSharedPtr<KisImage> KisImageWSP;

class KisPaintDevice;
typedef KisSharedPtr<KisPaintDevice> KisPaintDeviceSP;
typedef KisWeakSharedPtr<KisPaintDevice> KisPaintDeviceWSP;
typedef KisSharedPtrVector<KisPaintDevice> vKisPaintDeviceSP;
typedef vKisPaintDeviceSP::iterator vKisPaintDeviceSP_it;
typedef vKisPaintDeviceSP::const_iterator vKisPaintDeviceSP_cit;

class KisFixedPaintDevice;
typedef KisSharedPtr<KisFixedPaintDevice> KisFixedPaintDeviceSP;

class KisMask;
typedef KisSharedPtr<KisMask> KisMaskSP;
typedef KisWeakSharedPtr<KisMask> KisMaskWSP;
typedef KisSharedPtrVector<KisMask> vKisMaskSP;
typedef vKisMaskSP::iterator vKisMaskSP_it;
typedef vKisMaskSP::const_iterator vKisMaskSP_cit;

class KisNode;
typedef KisSharedPtr<KisNode> KisNodeSP;
typedef KisWeakSharedPtr<KisNode> KisNodeWSP;
typedef KisSharedPtrVector<KisNode> vKisNodeSP;
typedef vKisNodeSP::iterator vKisNodeSP_it;
typedef vKisNodeSP::const_iterator vKisNodeSP_cit;

class KisBaseNode;
typedef KisSharedPtr<KisBaseNode> KisBaseNodeSP;
typedef KisWeakSharedPtr<KisBaseNode> KisBaseNodeWSP;
typedef KisSharedPtrVector<KisBaseNode> vKisBaseNodeSP;
typedef vKisBaseNodeSP::iterator vKisBaseNodeSP_it;
typedef vKisBaseNodeSP::const_iterator vKisBaseNodeSP_cit;


class KisEffectMask;
typedef KisSharedPtr<KisEffectMask> KisEffectMaskSP;
typedef KisWeakSharedPtr<KisEffectMask> KisEffectMaskWSP;
typedef KisSharedPtrVector<KisEffectMask> vKisEffectMaskSP;
typedef vKisEffectMaskSP::iterator vKisEffectMaskSP_it;
typedef vKisEffectMaskSP::const_iterator vKisEffectMaskSP_cit;

class KisFilterMask;
typedef KisSharedPtr<KisFilterMask> KisFilterMaskSP;
typedef KisWeakSharedPtr<KisFilterMask> KisFilterMaskWSP;

class KisTransparencyMask;
typedef KisSharedPtr<KisTransparencyMask> KisTransparencyMaskSP;
typedef KisWeakSharedPtr<KisTransparencyMask> KisTransparencyMaskWSP;
typedef KisSharedPtrVector<KisTransparencyMask> vKisTransparencyMaskSP;
typedef vKisTransparencyMaskSP::iterator vKisTransparencyMaskSP_it;
typedef vKisTransparencyMaskSP::const_iterator vKisTransparencyMaskSP_cit;

class KisTransformationMask;
typedef KisSharedPtr<KisTransformationMask> KisTransformationMaskSP;
typedef KisWeakSharedPtr<KisTransformationMask> KisTransformationMaskWSP;
typedef KisSharedPtrVector<KisTransformationMask> vKisTransformationMaskSP;
typedef vKisTransformationMaskSP::iterator vKisTransformationMaskSP_it;
typedef vKisTransformationMaskSP::const_iterator vKisTransformationMaskSP_cit;


class KisLayer;
typedef KisSharedPtr<KisLayer> KisLayerSP;
typedef KisWeakSharedPtr<KisLayer> KisLayerWSP;
typedef KisSharedPtrVector<KisLayer> vKisLayerSP;
typedef vKisLayerSP::iterator vKisLayerSP_it;
typedef vKisLayerSP::const_iterator vKisLayerSP_cit;

class KisShapeLayer;
typedef KisSharedPtr<KisShapeLayer> KisShapeLayerSP;

class KisPaintLayer;
typedef KisSharedPtr<KisPaintLayer> KisPaintLayerSP;

class KisAdjustmentLayer;
typedef KisSharedPtr<KisAdjustmentLayer> KisAdjustmentLayerSP;

class KisGeneratorLayer;
typedef KisSharedPtr<KisGeneratorLayer> KisGeneratorLayerSP;

class KisCloneLayer;
typedef KisSharedPtr<KisCloneLayer> KisCloneLayerSP;

class KisGroupLayer;
typedef KisSharedPtr<KisGroupLayer> KisGroupLayerSP;
typedef KisWeakSharedPtr<KisGroupLayer> KisGroupLayerWSP;

class KisSelection;
typedef KisSharedPtr<KisSelection> KisSelectionSP;

class KisBackground;
typedef KisSharedPtr<KisBackground> KisBackgroundSP;

class KisSelectionMask;
typedef KisSharedPtr<KisSelectionMask> KisSelectionMaskSP;

class KisPixelSelection;
typedef KisSharedPtr<KisPixelSelection> KisPixelSelectionSP;

class KisHistogram;
typedef KisSharedPtr<KisHistogram> KisHistogramSP;

typedef QVector<QPoint> vKisSegments;

class KisFilter;
typedef KisSharedPtr<KisFilter> KisFilterSP;

class KisGenerator;
typedef KisSharedPtr<KisGenerator> KisGeneratorSP;

class KisConvolutionKernel;
typedef KisSharedPtr<KisConvolutionKernel> KisConvolutionKernelSP;

class KisAnnotation;
typedef KisSharedPtr<KisAnnotation> KisAnnotationSP;
typedef KisSharedPtrVector<KisAnnotation> vKisAnnotationSP;
typedef vKisAnnotationSP::iterator vKisAnnotationSP_it;
typedef vKisAnnotationSP::const_iterator vKisAnnotationSP_cit;

// #include <kis_iterators_pixel.h>
template<class T, typename TSelect> class KisLineIteratorPixelBase;
class KisHLineConstIterator;
typedef KisLineIteratorPixelBase<KisHLineConstIterator, const quint8*> KisHLineConstIteratorPixel;
class KisHLineIterator;
typedef KisLineIteratorPixelBase<KisHLineIterator, quint8*> KisHLineIteratorPixel;
class KisVLineConstIterator;
typedef KisLineIteratorPixelBase<KisVLineConstIterator, const quint8*> KisVLineConstIteratorPixel;
class KisVLineIterator;
typedef KisLineIteratorPixelBase<KisVLineIterator, quint8*> KisVLineIteratorPixel;
template<class T, typename TSelect> class KisRectIteratorPixelBase;
class KisRectConstIterator;
typedef KisRectIteratorPixelBase<KisRectConstIterator, const quint8*> KisRectConstIteratorPixel;
class KisRectIterator;
typedef KisRectIteratorPixelBase<KisRectIterator, quint8*> KisRectIteratorPixel;

// Repeat iterators
template<class T> class KisRepeatHLineIteratorPixelBase;
typedef KisRepeatHLineIteratorPixelBase< KisHLineConstIteratorPixel > KisRepeatHLineConstIteratorPixel;
template<class T> class KisRepeatVLineIteratorPixelBase;
typedef KisRepeatVLineIteratorPixelBase< KisVLineConstIteratorPixel > KisRepeatVLineConstIteratorPixel;

// Random accessor
template<class T, typename TSelect> class KisRandomAccessorPixelBase;
class KisRandomConstAccessor;
typedef KisRandomAccessorPixelBase<KisRandomConstAccessor, const quint8*> KisRandomConstAccessorPixel;
class KisRandomAccessor;
typedef KisRandomAccessorPixelBase<KisRandomAccessor, quint8*> KisRandomAccessorPixel;

typedef QVector<QPointF> vQPointF;

class KisPaintOpPreset;
typedef KisSharedPtr<KisPaintOpPreset> KisPaintOpPresetSP;

class KisPaintOpSettings;
typedef KisSharedPtr<KisPaintOpSettings> KisPaintOpSettingsSP;

#include <kis_shared_ptr.h>

#endif // KISTYPES_H_

