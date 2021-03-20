/*
 *  SPDX-FileCopyrightText: 2002 Patrick Julien <freak@codepimps.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISTYPES_H_
#define KISTYPES_H_

#include <QVector>
#include <QPoint>
#include <QList>

#include "kritaimage_export.h"

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
 * These types should never be passed by reference,
 * because that will mess up their reference counter.
 *
 * An example of the naming pattern used:
 *
 * KisPaintDeviceSP is a KisSharedPtr of KisPaintDevice
 * KisPaintDeviceWSP is a KisWeakSharedPtr of KisPaintDevice
 * vKisPaintDeviceSP is a QVector of KisPaintDeviceSP
 * vKisPaintDeviceSP_it is an iterator of vKisPaintDeviceSP
 *
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

class KisColorizeMask;
typedef KisSharedPtr<KisColorizeMask> KisColorizeMaskSP;
typedef KisWeakSharedPtr<KisColorizeMask> KisColorizeMaskWSP;

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

class KisFileLayer;
typedef KisSharedPtr<KisFileLayer> KisFileLayerSP;
typedef KisWeakSharedPtr<KisFileLayer> KisFileLayerWSP;

class KisSelection;
typedef KisSharedPtr<KisSelection> KisSelectionSP;
typedef KisWeakSharedPtr<KisSelection> KisSelectionWSP;

class KisSelectionComponent;
typedef KisSharedPtr<KisSelectionComponent> KisSelectionComponentSP;

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
typedef KisWeakSharedPtr<KisAnimationFrameCache> KisAnimationFrameCacheWSP;

class KisPaintingAssistant;
typedef QSharedPointer<KisPaintingAssistant> KisPaintingAssistantSP;
typedef QWeakPointer<KisPaintingAssistant> KisPaintingAssistantWSP;

class KisReferenceImage;
typedef QSharedPointer<KisReferenceImage> KisReferenceImageSP;
typedef QWeakPointer<KisReferenceImage> KisReferenceImageWSP;

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
typedef QSharedPointer<KisPaintOpPreset> KisPaintOpPresetSP;
typedef QWeakPointer<KisPaintOpPreset> KisPaintOpPresetWSP;

template <typename T>
class KisPinnedSharedPtr;

class KisPaintOpSettings;
typedef KisPinnedSharedPtr<KisPaintOpSettings> KisPaintOpSettingsSP;

template <typename T>
class KisRestrictedSharedPtr;
typedef KisRestrictedSharedPtr<KisPaintOpSettings> KisPaintOpSettingsRestrictedSP;

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

typedef QList<KisPaintDeviceSP> KisPaintDeviceList;

class KisStroke;
typedef QSharedPointer<KisStroke> KisStrokeSP;
typedef QWeakPointer<KisStroke> KisStrokeWSP;
typedef KisStrokeWSP KisStrokeId;

class KisFilterConfiguration;
typedef KisPinnedSharedPtr<KisFilterConfiguration> KisFilterConfigurationSP;

class KisPropertiesConfiguration;
typedef KisPinnedSharedPtr<KisPropertiesConfiguration> KisPropertiesConfigurationSP;

class KisLockedProperties;
typedef KisSharedPtr<KisLockedProperties> KisLockedPropertiesSP;

class KisProjectionUpdatesFilter;
typedef QSharedPointer<KisProjectionUpdatesFilter> KisProjectionUpdatesFilterSP;
using KisProjectionUpdatesFilterCookie = void*;

class KisAbstractProjectionPlane;
typedef QSharedPointer<KisAbstractProjectionPlane> KisAbstractProjectionPlaneSP;
typedef QWeakPointer<KisAbstractProjectionPlane> KisAbstractProjectionPlaneWSP;

class KisProjectionLeaf;
typedef QSharedPointer<KisProjectionLeaf> KisProjectionLeafSP;
typedef QWeakPointer<KisProjectionLeaf> KisProjectionLeafWSP;

class KisKeyframe;
typedef QSharedPointer<KisKeyframe> KisKeyframeSP;
typedef QWeakPointer<KisKeyframe> KisKeyframeWSP;

class KisScalarKeyframe;
typedef QSharedPointer<KisScalarKeyframe> KisScalarKeyframeSP;
typedef QWeakPointer<KisScalarKeyframe> KisScalarKeyframeWSP;

class KisRasterKeyframe;
typedef QSharedPointer<KisRasterKeyframe> KisRasterKeyframeSP;
typedef QWeakPointer<KisRasterKeyframe> KisRasterKeyframeWSP;

class KisFilterChain;
typedef KisSharedPtr<KisFilterChain> KisFilterChainSP;

class KisProofingConfiguration;
typedef QSharedPointer<KisProofingConfiguration> KisProofingConfigurationSP;
typedef QWeakPointer<KisProofingConfiguration> KisProofingConfigurationWSP;

class KisLayerComposition;
typedef QSharedPointer<KisLayerComposition> KisLayerCompositionSP;
typedef QWeakPointer<KisLayerComposition> KisLayerCompositionWSP;

class KisMirrorAxis;
typedef KisSharedPtr<KisMirrorAxis> KisMirrorAxisSP;
typedef KisWeakSharedPtr<KisMirrorAxis> KisMirrorAxisWSP;

class StoryboardItem;
typedef QSharedPointer<StoryboardItem> StoryboardItemSP;
typedef QVector<StoryboardItemSP> StoryboardItemList;

#include <QSharedPointer>
#include <QWeakPointer>
#include <kis_shared_ptr.h>
#include <kis_restricted_shared_ptr.h>
#include <kis_pinned_shared_ptr.h>

#endif // KISTYPES_H_

