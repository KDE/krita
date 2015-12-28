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
#include <QList>

template<class T>
class KisWeakSharedPtr;
template<class T>
class KisSharedPtr;

template<class T> class QSharedPointer;
template<class T> class QWeakPointer;

template <class T>
uint qHash(KisSharedPtr<T> ptr) {
    return qHash(ptr.data());
}

template <class T>
uint qHash(KisWeakSharedPtr<T> ptr) {
    return qHash(ptr.data());
}

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
typedef QVector<KisPaintDeviceSP> vKisPaintDeviceSP;
typedef vKisPaintDeviceSP::iterator vKisPaintDeviceSP_it;

class KisFixedPaintDevice;
typedef KisSharedPtr<KisFixedPaintDevice> KisFixedPaintDeviceSP;

class KisMask;
typedef KisSharedPtr<KisMask> KisMaskSP;
typedef KisWeakSharedPtr<KisMask> KisMaskWSP;

class KisNode;
typedef KisSharedPtr<KisNode> KisNodeSP;
typedef KisWeakSharedPtr<KisNode> KisNodeWSP;
typedef QVector<KisNodeSP> vKisNodeSP;
typedef vKisNodeSP::iterator vKisNodeSP_it;
typedef vKisNodeSP::const_iterator vKisNodeSP_cit;

class KisBaseNode;
typedef KisSharedPtr<KisBaseNode> KisBaseNodeSP;
typedef KisWeakSharedPtr<KisBaseNode> KisBaseNodeWSP;

class KisEffectMask;
typedef KisSharedPtr<KisEffectMask> KisEffectMaskSP;
typedef KisWeakSharedPtr<KisEffectMask> KisEffectMaskWSP;

class KisFilterMask;
typedef KisSharedPtr<KisFilterMask> KisFilterMaskSP;
typedef KisWeakSharedPtr<KisFilterMask> KisFilterMaskWSP;

class KisTransformMask;
typedef KisSharedPtr<KisTransformMask> KisTransformMaskSP;
typedef KisWeakSharedPtr<KisTransformMask> KisTransformMaskWSP;

class KisTransformMaskParamsInterface;
typedef QSharedPointer<KisTransformMaskParamsInterface> KisTransformMaskParamsInterfaceSP;
typedef QWeakPointer<KisTransformMaskParamsInterface> KisTransformMaskParamsInterfaceWSP;

class KisTransparencyMask;
typedef KisSharedPtr<KisTransparencyMask> KisTransparencyMaskSP;
typedef KisWeakSharedPtr<KisTransparencyMask> KisTransparencyMaskWSP;

class KisLayer;
typedef KisSharedPtr<KisLayer> KisLayerSP;
typedef KisWeakSharedPtr<KisLayer> KisLayerWSP;

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
typedef KisWeakSharedPtr<KisCloneLayer> KisCloneLayerWSP;

class KisGroupLayer;
typedef KisSharedPtr<KisGroupLayer> KisGroupLayerSP;
typedef KisWeakSharedPtr<KisGroupLayer> KisGroupLayerWSP;

class KisSelection;
typedef KisSharedPtr<KisSelection> KisSelectionSP;
typedef KisWeakSharedPtr<KisSelection> KisSelectionWSP;

class KisSelectionComponent;
typedef KisSharedPtr<KisSelectionComponent> KisSelectionComponentSP;

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

class KisLayerStyleFilter;
typedef KisSharedPtr<KisLayerStyleFilter> KisLayerStyleFilterSP;

class KisGenerator;
typedef KisSharedPtr<KisGenerator> KisGeneratorSP;

class KisConvolutionKernel;
typedef KisSharedPtr<KisConvolutionKernel> KisConvolutionKernelSP;

class KisAnnotation;
typedef KisSharedPtr<KisAnnotation> KisAnnotationSP;
typedef QVector<KisAnnotationSP> vKisAnnotationSP;
typedef vKisAnnotationSP::iterator vKisAnnotationSP_it;
typedef vKisAnnotationSP::const_iterator vKisAnnotationSP_cit;

class KisAnimationFrameCache;
typedef KisSharedPtr<KisAnimationFrameCache> KisAnimationFrameCacheSP;

// Repeat iterators
class KisHLineIterator2;
template<class T> class KisRepeatHLineIteratorPixelBase;
typedef KisRepeatHLineIteratorPixelBase< KisHLineIterator2 > KisRepeatHLineConstIteratorNG;
typedef KisSharedPtr<KisRepeatHLineConstIteratorNG> KisRepeatHLineConstIteratorSP;

class KisVLineIterator2;
template<class T> class KisRepeatVLineIteratorPixelBase;
typedef KisRepeatVLineIteratorPixelBase< KisVLineIterator2 > KisRepeatVLineConstIteratorNG;
typedef KisSharedPtr<KisRepeatVLineConstIteratorNG> KisRepeatVLineConstIteratorSP;


// NG Iterators
class KisHLineIteratorNG;
typedef KisSharedPtr<KisHLineIteratorNG> KisHLineIteratorSP;

class KisHLineConstIteratorNG;
typedef KisSharedPtr<KisHLineConstIteratorNG> KisHLineConstIteratorSP;

class KisVLineIteratorNG;
typedef KisSharedPtr<KisVLineIteratorNG> KisVLineIteratorSP;

class KisVLineConstIteratorNG;
typedef KisSharedPtr<KisVLineConstIteratorNG> KisVLineConstIteratorSP;

class KisRandomConstAccessorNG;
typedef KisSharedPtr<KisRandomConstAccessorNG> KisRandomConstAccessorSP;

class KisRandomAccessorNG;
typedef KisSharedPtr<KisRandomAccessorNG> KisRandomAccessorSP;

class KisRandomSubAccessor;
typedef KisSharedPtr<KisRandomSubAccessor> KisRandomSubAccessorSP;

// Things

typedef QVector<QPointF> vQPointF;

class KisPaintOpPreset;
typedef KisSharedPtr<KisPaintOpPreset> KisPaintOpPresetSP;
typedef KisWeakSharedPtr<KisPaintOpPreset> KisPaintOpPresetWSP;

class KisPaintOpSettings;
typedef KisSharedPtr<KisPaintOpSettings> KisPaintOpSettingsSP;

class KisPaintOp;
typedef KisSharedPtr<KisPaintOp> KisPaintOpSP;

class KoID;
typedef QList<KoID> KoIDList;

class KoUpdater;
template<class T> class QPointer;
typedef QPointer<KoUpdater> KoUpdaterPtr;

class KisProcessingVisitor;
typedef KisSharedPtr<KisProcessingVisitor> KisProcessingVisitorSP;

class KUndo2Command;
typedef QSharedPointer<KUndo2Command> KUndo2CommandSP;

typedef QList<KisNodeSP> KisNodeList;
typedef QSharedPointer<KisNodeList> KisNodeListSP;

class KisStroke;
typedef QSharedPointer<KisStroke> KisStrokeSP;
typedef QWeakPointer<KisStroke> KisStrokeWSP;
typedef KisStrokeWSP KisStrokeId;

class KisFilterConfiguration;
typedef QSharedPointer<KisFilterConfiguration> KisSafeFilterConfigurationSP;

class KisProjectionUpdatesFilter;
typedef QSharedPointer<KisProjectionUpdatesFilter> KisProjectionUpdatesFilterSP;

class KisAbstractProjectionPlane;
typedef QSharedPointer<KisAbstractProjectionPlane> KisAbstractProjectionPlaneSP;
typedef QWeakPointer<KisAbstractProjectionPlane> KisAbstractProjectionPlaneWSP;

class KisProjectionLeaf;
typedef QSharedPointer<KisProjectionLeaf> KisProjectionLeafSP;
typedef QWeakPointer<KisProjectionLeaf> KisProjectionLeafWSP;

class KisPSDLayerStyle;
typedef QSharedPointer<KisPSDLayerStyle> KisPSDLayerStyleSP;

class KoAbstractGradient;
typedef QSharedPointer<KoAbstractGradient> KoAbstractGradientSP;

class KisKeyframe;
typedef QSharedPointer<KisKeyframe> KisKeyframeSP;
typedef QWeakPointer<KisKeyframe> KisKeyframeWSP;

#include <QSharedPointer>
#include <QWeakPointer>
#include <kis_shared_ptr.h>

#endif // KISTYPES_H_

