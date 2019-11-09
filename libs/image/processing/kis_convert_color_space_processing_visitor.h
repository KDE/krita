/*
 *  Copyright (c) 2011 Dmitry Kazakov <dimula73@gmail.com>
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
                                          KoColorConversionTransformation::ConversionFlags conversionFlags);

private:
    void visitNodeWithPaintDevice(KisNode *node, KisUndoAdapter *undoAdapter) override;
    void visitExternalLayer(KisExternalLayer *layer, KisUndoAdapter *undoAdapter) override;

public:

    void visit(KisTransformMask *mask, KisUndoAdapter *undoAdapter) override;
    void visitColorizeMask(KisColorizeMask *mask, KisUndoAdapter *undoAdapter) override;
    using KisSimpleProcessingVisitor::visit;

private:
    const KoColorSpace *m_srcColorSpace;
    const KoColorSpace *m_dstColorSpace;
    KoColorConversionTransformation::Intent m_renderingIntent;
    KoColorConversionTransformation::ConversionFlags m_conversionFlags;
};

#endif /* __KIS_CONVERT_COLORSPACE_PROCESSING_VISITOR_H */
