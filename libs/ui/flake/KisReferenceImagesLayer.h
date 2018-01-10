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

#ifndef KRITA_KISREFERENCEIMAGESLAYER_H
#define KRITA_KISREFERENCEIMAGESLAYER_H

#include "kis_shape_layer.h"

#include <kis_types.h>

class KRITAUI_EXPORT KisReferenceImagesLayer : public KisShapeLayer
{
    Q_OBJECT

public:
    KisReferenceImagesLayer(KoShapeBasedDocumentBase* shapeController, KisImageWSP image);

    KUndo2Command * addReferenceImage(KisReferenceImage *referenceImage);

    bool allowAsChild(KisNodeSP) const override;

    bool accept(KisNodeVisitor&) override;
    void accept(KisProcessingVisitor &visitor, KisUndoAdapter *undoAdapter) override;

    friend struct AddReferenceImageCommand;
};


#endif //KRITA_KISREFERENCEIMAGESLAYER_H
