/*
 * Copyright (C) 2017 Jouni Pentikäinen <joupent@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include <kis_node_visitor.h>
#include <kis_processing_visitor.h>

#include "KisReferenceImagesLayer.h"

KisReferenceImagesLayer::KisReferenceImagesLayer(KoShapeBasedDocumentBase* shapeController, KisImageWSP image)
    : KisShapeLayer(shapeController, image, i18n("Reference images"), OPACITY_OPAQUE_U8)
{}

void KisReferenceImagesLayer::addReferenceImage(KisReferenceImageSP referenceImage)
{
    // TODO
}

bool KisReferenceImagesLayer::allowAsChild(KisNodeSP) const
{
    return false;
}

bool KisReferenceImagesLayer::accept(KisNodeVisitor &visitor)
{
    return visitor.visit(this);
}

void KisReferenceImagesLayer::accept(KisProcessingVisitor &visitor, KisUndoAdapter *undoAdapter) 
{
    visitor.visit(this, undoAdapter);
}
