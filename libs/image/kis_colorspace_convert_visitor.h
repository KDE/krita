/*
 *  Copyright (c) 2005 C. Boemann <cbo@boemann.dk>
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
#ifndef KIS_COLORSPACE_CONVERT_VISITOR_H_
#define KIS_COLORSPACE_CONVERT_VISITOR_H_


#include <KoColorConversionTransformation.h>
#include <KoColorSpace.h>

#include <kritaimage_export.h>
#include "kis_types.h"
#include "kis_node_visitor.h"


/**
 * This will convert all layers to the destination color space.
 */
class KRITAIMAGE_EXPORT KisColorSpaceConvertVisitor : public KisNodeVisitor
{
public:
    KisColorSpaceConvertVisitor(KisImageWSP image,
                                const KoColorSpace *srcColorSpace,
                                const KoColorSpace *dstColorSpace,
                                KoColorConversionTransformation::Intent renderingIntent,
                                KoColorConversionTransformation::ConversionFlags conversionFlags);
    ~KisColorSpaceConvertVisitor() override;

public:

    bool visit(KisPaintLayer *layer) override;
    bool visit(KisGroupLayer *layer) override;
    bool visit(KisAdjustmentLayer* layer) override;
    bool visit(KisGeneratorLayer * layer) override;

    bool visit(KisExternalLayer *) override {
        return true;
    }

    bool visit(KisNode*) override {
        return true;
    }

    bool visit(KisCloneLayer*) override {
        return true;
    }

    bool visit(KisFilterMask*) override {
        return true;
    }

    bool visit(KisTransformMask*) override {
        return true;
    }

    bool visit(KisTransparencyMask*) override {
        return true;
    }

    bool visit(KisSelectionMask*) override {
        return true;
    }

    bool visit(KisColorizeMask *mask) override;

private:

    bool convertPaintDevice(KisLayer* layer);

    KisImageWSP m_image;
    const KoColorSpace *m_srcColorSpace;
    const KoColorSpace *m_dstColorSpace;
    KoColorConversionTransformation::Intent m_renderingIntent;
    KoColorConversionTransformation::ConversionFlags m_conversionFlags;
    QBitArray m_emptyChannelFlags;
};


#endif // KIS_COLORSPACE_CONVERT_VISITOR_H_

