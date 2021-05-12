/*
 *  SPDX-FileCopyrightText: 2017 Dmitry Kazakov <dimula73@gmail.com>
 *  SPDX-FileCopyrightText: 2021 Deif Lou <ginoba@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisMaskingBrushCompositeOpFactory.h"

#include "kis_assert.h"

#include <KoCompositeOpRegistry.h>
#include <KoCompositeOpFunctions.h>

#include "KisMaskingBrushCompositeOp.h"

#include <KoConfig.h>
#ifdef HAVE_OPENEXR
#include <half.h>
#endif /* HAVE_OPENEXR */


namespace {

template <typename channel_type, bool mask_is_alpha = false>
KisMaskingBrushCompositeOpBase *createTypedOp(const QString &id, int pixelSize, int alphaOffset)
{
    KisMaskingBrushCompositeOpBase *result = 0;

    if (id == COMPOSITE_MULT) {
        result = new KisMaskingBrushCompositeOp<channel_type, KIS_MASKING_BRUSH_COMPOSITE_MULT, mask_is_alpha, false>(pixelSize, alphaOffset);
    } else if (id == COMPOSITE_DARKEN) {
        result = new KisMaskingBrushCompositeOp<channel_type, KIS_MASKING_BRUSH_COMPOSITE_DARKEN, mask_is_alpha, false>(pixelSize, alphaOffset);
    } else if (id == COMPOSITE_OVERLAY) {
        result = new KisMaskingBrushCompositeOp<channel_type, KIS_MASKING_BRUSH_COMPOSITE_OVERLAY, mask_is_alpha, false>(pixelSize, alphaOffset);
    } else if (id == COMPOSITE_DODGE) {
        result = new KisMaskingBrushCompositeOp<channel_type, KIS_MASKING_BRUSH_COMPOSITE_DODGE, mask_is_alpha, false>(pixelSize, alphaOffset);
    } else if (id == COMPOSITE_BURN) {
        result = new KisMaskingBrushCompositeOp<channel_type, KIS_MASKING_BRUSH_COMPOSITE_BURN, mask_is_alpha, false>(pixelSize, alphaOffset);
    } else if (id == COMPOSITE_LINEAR_BURN) {
        result = new KisMaskingBrushCompositeOp<channel_type, KIS_MASKING_BRUSH_COMPOSITE_LINEAR_BURN, mask_is_alpha, false>(pixelSize, alphaOffset);
    } else if (id == COMPOSITE_LINEAR_DODGE) {
        result = new KisMaskingBrushCompositeOp<channel_type, KIS_MASKING_BRUSH_COMPOSITE_LINEAR_DODGE, mask_is_alpha, false>(pixelSize, alphaOffset);
    } else if (id == COMPOSITE_HARD_MIX_PHOTOSHOP) {
        result = new KisMaskingBrushCompositeOp<channel_type, KIS_MASKING_BRUSH_COMPOSITE_HARD_MIX_PHOTOSHOP, mask_is_alpha, false>(pixelSize, alphaOffset);
    } else if (id == COMPOSITE_HARD_MIX_SOFTER_PHOTOSHOP) {
        result = new KisMaskingBrushCompositeOp<channel_type, KIS_MASKING_BRUSH_COMPOSITE_HARD_MIX_SOFTER_PHOTOSHOP, mask_is_alpha, false>(pixelSize, alphaOffset);
    } else if (id == COMPOSITE_SUBTRACT) {
        result = new KisMaskingBrushCompositeOp<channel_type, KIS_MASKING_BRUSH_COMPOSITE_SUBTRACT, mask_is_alpha, false>(pixelSize, alphaOffset);
    }

    KIS_SAFE_ASSERT_RECOVER (result && "Unknown composite op for masked brush!") {
        result = new KisMaskingBrushCompositeOp<channel_type, KIS_MASKING_BRUSH_COMPOSITE_MULT, mask_is_alpha, false>(pixelSize, alphaOffset);
    }

    return result;
}

template <typename channel_type, bool mask_is_alpha = false>
KisMaskingBrushCompositeOpBase *createTypedOp(const QString &id, int pixelSize, int alphaOffset, qreal strength)
{
    KisMaskingBrushCompositeOpBase *result = 0;

    if (id == COMPOSITE_MULT) {
        result = new KisMaskingBrushCompositeOp<channel_type, KIS_MASKING_BRUSH_COMPOSITE_MULT, mask_is_alpha, true>(pixelSize, alphaOffset, strength);
    } else if (id == COMPOSITE_DARKEN) {
        result = new KisMaskingBrushCompositeOp<channel_type, KIS_MASKING_BRUSH_COMPOSITE_DARKEN, mask_is_alpha, true>(pixelSize, alphaOffset, strength);
    } else if (id == COMPOSITE_OVERLAY) {
        result = new KisMaskingBrushCompositeOp<channel_type, KIS_MASKING_BRUSH_COMPOSITE_OVERLAY, mask_is_alpha, true>(pixelSize, alphaOffset, strength);
    } else if (id == COMPOSITE_DODGE) {
        result = new KisMaskingBrushCompositeOp<channel_type, KIS_MASKING_BRUSH_COMPOSITE_DODGE, mask_is_alpha, true>(pixelSize, alphaOffset, strength);
    } else if (id == COMPOSITE_BURN) {
        result = new KisMaskingBrushCompositeOp<channel_type, KIS_MASKING_BRUSH_COMPOSITE_BURN, mask_is_alpha, true>(pixelSize, alphaOffset, strength);
    } else if (id == COMPOSITE_LINEAR_BURN) {
        result = new KisMaskingBrushCompositeOp<channel_type, KIS_MASKING_BRUSH_COMPOSITE_LINEAR_BURN, mask_is_alpha, true>(pixelSize, alphaOffset, strength);
    } else if (id == COMPOSITE_LINEAR_DODGE) {
        result = new KisMaskingBrushCompositeOp<channel_type, KIS_MASKING_BRUSH_COMPOSITE_LINEAR_DODGE, mask_is_alpha, true>(pixelSize, alphaOffset, strength);
    } else if (id == COMPOSITE_HARD_MIX_PHOTOSHOP) {
        result = new KisMaskingBrushCompositeOp<channel_type, KIS_MASKING_BRUSH_COMPOSITE_HARD_MIX_PHOTOSHOP, mask_is_alpha, true>(pixelSize, alphaOffset, strength);
    } else if (id == COMPOSITE_HARD_MIX_SOFTER_PHOTOSHOP) {
        result = new KisMaskingBrushCompositeOp<channel_type, KIS_MASKING_BRUSH_COMPOSITE_HARD_MIX_SOFTER_PHOTOSHOP, mask_is_alpha, true>(pixelSize, alphaOffset, strength);
    } else if (id == COMPOSITE_SUBTRACT) {
        result = new KisMaskingBrushCompositeOp<channel_type, KIS_MASKING_BRUSH_COMPOSITE_SUBTRACT, mask_is_alpha, true>(pixelSize, alphaOffset, strength);
    // Special modes that useful when used with strength
    } else if (id == "height") {
        result = new KisMaskingBrushCompositeOp<channel_type, KIS_MASKING_BRUSH_COMPOSITE_HEIGHT, mask_is_alpha, true>(pixelSize, alphaOffset, strength);
    } else if (id == "linear_height") {
        result = new KisMaskingBrushCompositeOp<channel_type, KIS_MASKING_BRUSH_COMPOSITE_LINEAR_HEIGHT, mask_is_alpha, true>(pixelSize, alphaOffset, strength);
    } else if (id == "height_photoshop") {
        result = new KisMaskingBrushCompositeOp<channel_type, KIS_MASKING_BRUSH_COMPOSITE_HEIGHT_PHOTOSHOP, mask_is_alpha, true>(pixelSize, alphaOffset, strength);
    } else if (id == "linear_height_photoshop") {
        result = new KisMaskingBrushCompositeOp<channel_type, KIS_MASKING_BRUSH_COMPOSITE_LINEAR_HEIGHT_PHOTOSHOP, mask_is_alpha, true>(pixelSize, alphaOffset, strength);
    }

    KIS_SAFE_ASSERT_RECOVER (result && "Unknown composite op for masked brush!") {
        result = new KisMaskingBrushCompositeOp<channel_type, KIS_MASKING_BRUSH_COMPOSITE_MULT, mask_is_alpha, true>(pixelSize, alphaOffset, strength);
    }

    return result;
}

}

template <bool mask_is_alpha>
KisMaskingBrushCompositeOpBase *createImpl(const QString &id, KoChannelInfo::enumChannelValueType channelType, int pixelSize, int alphaOffset)
{
    KisMaskingBrushCompositeOpBase *result = 0;

    switch (channelType) {
    case KoChannelInfo::UINT8:
        result = createTypedOp<quint8, mask_is_alpha>(id, pixelSize, alphaOffset);
        break;
    case KoChannelInfo::UINT16:
        result = createTypedOp<quint16, mask_is_alpha>(id, pixelSize, alphaOffset);
        break;
    case KoChannelInfo::UINT32:
        result = createTypedOp<quint32, mask_is_alpha>(id, pixelSize, alphaOffset);
        break;

#ifdef HAVE_OPENEXR
    case KoChannelInfo::FLOAT16:
        result = createTypedOp<half, mask_is_alpha>(id, pixelSize, alphaOffset);
        break;
#endif /* HAVE_OPENEXR */

    case KoChannelInfo::FLOAT32:
        result = createTypedOp<float, mask_is_alpha>(id, pixelSize, alphaOffset);
        break;
    case KoChannelInfo::FLOAT64:
        result = createTypedOp<double, mask_is_alpha>(id, pixelSize, alphaOffset);
        break;
//    NOTE: we have no color space like that, so it is not supported!
//    case KoChannelInfo::INT8:
//        result = createTypedOp<qint8>(id, pixelSize, alphaOffset);
//        break;
    case KoChannelInfo::INT16:
        result = createTypedOp<qint16, mask_is_alpha>(id, pixelSize, alphaOffset);
        break;
    default:
        KIS_SAFE_ASSERT_RECOVER_NOOP(0 && "Unknown channel type for masked brush!");
    }

    return result;
}

template <bool mask_is_alpha>
KisMaskingBrushCompositeOpBase *createImpl(const QString &id, KoChannelInfo::enumChannelValueType channelType, int pixelSize, int alphaOffset, qreal strength)
{
    KisMaskingBrushCompositeOpBase *result = 0;

    switch (channelType) {
    case KoChannelInfo::UINT8:
        result = createTypedOp<quint8, mask_is_alpha>(id, pixelSize, alphaOffset, strength);
        break;
    case KoChannelInfo::UINT16:
        result = createTypedOp<quint16, mask_is_alpha>(id, pixelSize, alphaOffset, strength);
        break;
    case KoChannelInfo::UINT32:
        result = createTypedOp<quint32, mask_is_alpha>(id, pixelSize, alphaOffset, strength);
        break;

#ifdef HAVE_OPENEXR
    case KoChannelInfo::FLOAT16:
        result = createTypedOp<half, mask_is_alpha>(id, pixelSize, alphaOffset, strength);
        break;
#endif /* HAVE_OPENEXR */

    case KoChannelInfo::FLOAT32:
        result = createTypedOp<float, mask_is_alpha>(id, pixelSize, alphaOffset, strength);
        break;
    case KoChannelInfo::FLOAT64:
        result = createTypedOp<double, mask_is_alpha>(id, pixelSize, alphaOffset, strength);
        break;
    case KoChannelInfo::INT16:
        result = createTypedOp<qint16, mask_is_alpha>(id, pixelSize, alphaOffset, strength);
        break;
    default:
        KIS_SAFE_ASSERT_RECOVER_NOOP(0 && "Unknown channel type for masked brush!");
    }

    return result;
}

KisMaskingBrushCompositeOpBase *KisMaskingBrushCompositeOpFactory::create(const QString &id, KoChannelInfo::enumChannelValueType channelType, int pixelSize, int alphaOffset)
{
    return createImpl<false>(id, channelType, pixelSize, alphaOffset);
}

KisMaskingBrushCompositeOpBase *KisMaskingBrushCompositeOpFactory::create(const QString &id, KoChannelInfo::enumChannelValueType channelType, int pixelSize, int alphaOffset, qreal strength)
{
    return createImpl<false>(id, channelType, pixelSize, alphaOffset, strength);
}

KisMaskingBrushCompositeOpBase *KisMaskingBrushCompositeOpFactory::createForAlphaSrc(const QString &id, KoChannelInfo::enumChannelValueType channelType, int pixelSize, int alphaOffset)
{
    return createImpl<true>(id, channelType, pixelSize, alphaOffset);
}

KisMaskingBrushCompositeOpBase *KisMaskingBrushCompositeOpFactory::createForAlphaSrc(const QString &id, KoChannelInfo::enumChannelValueType channelType, int pixelSize, int alphaOffset, qreal strength)
{
    return createImpl<true>(id, channelType, pixelSize, alphaOffset, strength);
}

QStringList KisMaskingBrushCompositeOpFactory::supportedCompositeOpIds()
{
    QStringList ids;
    ids << COMPOSITE_MULT;
    ids << COMPOSITE_DARKEN;
    ids << COMPOSITE_OVERLAY;
    ids << COMPOSITE_DODGE;
    ids << COMPOSITE_BURN;
    ids << COMPOSITE_LINEAR_BURN;
    ids << COMPOSITE_LINEAR_DODGE;
    ids << COMPOSITE_HARD_MIX_PHOTOSHOP;
    ids << COMPOSITE_HARD_MIX_SOFTER_PHOTOSHOP;
    ids << COMPOSITE_SUBTRACT;

    return ids;
}
