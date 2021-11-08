/*
 *  SPDX-FileCopyrightText: 2011 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_CONVERT_COLORSPACE_PROCESSING_VISITOR_H
#define __KIS_CONVERT_COLORSPACE_PROCESSING_VISITOR_H

#include "kis_simple_processing_visitor.h"
#include <QRect>
#include "kis_types.h"
#include <KoColorConversionTransformation.h>

class KoColorSpace;

class KRITAIMAGE_EXPORT  KisConvertColorSpaceProcessingVisitor : public KisSimpleProcessingVisitor
{
public:
    KisConvertColorSpaceProcessingVisitor(const KoColorSpace *srcColorSpace,
                                          const KoColorSpace *dstColorSpace,
                                          KoColorConversionTransformation::Intent renderingIntent,
                                          KoColorConversionTransformation::ConversionFlags conversionFlags,
                                          bool convertImage, bool convertLayers);

private:
    void visitNodeWithPaintDevice(KisNode *node, KisUndoAdapter *undoAdapter) override;
    void visitExternalLayer(KisExternalLayer *layer, KisUndoAdapter *undoAdapter) override;

public:

    void visit(KisGroupLayer *layer, KisUndoAdapter *undoAdapter) override;
    void visit(KisTransformMask *mask, KisUndoAdapter *undoAdapter) override;
    void visitColorizeMask(KisColorizeMask *mask, KisUndoAdapter *undoAdapter) override;
    using KisSimpleProcessingVisitor::visit;

private:
    const KoColorSpace *m_srcColorSpace;
    const KoColorSpace *m_dstColorSpace;
    KoColorConversionTransformation::Intent m_renderingIntent;
    KoColorConversionTransformation::ConversionFlags m_conversionFlags;
    bool m_convertImage;
    bool m_convertLayers;
};

#endif /* __KIS_CONVERT_COLORSPACE_PROCESSING_VISITOR_H */
