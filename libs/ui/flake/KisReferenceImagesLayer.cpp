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

#include <KoShapeCreateCommand.h>
#include <kis_node_visitor.h>
#include <kis_processing_visitor.h>
#include <kis_shape_layer_canvas.h>

#include "KisReferenceImagesLayer.h"
#include "KisReferenceImage.h"

struct AddReferenceImageCommand : KoShapeCreateCommand
{
    AddReferenceImageCommand(KisReferenceImagesLayer *layer, KisReferenceImage* referenceImage)
        : KoShapeCreateCommand(layer->shapeController(), {referenceImage}, layer, nullptr, kundo2_i18n("Add reference image"))
        , m_layer(layer)
    {}

    void redo() override {
        KoShapeCreateCommand::redo();
    }

    void undo() override {
        KoShapeCreateCommand::undo();
    }

private:
    KisReferenceImagesLayer *m_layer;
};

class ReferenceImagesCanvas : public KisShapeLayerCanvasBase
{
public:
    ReferenceImagesCanvas(KisReferenceImagesLayer *parent, KisImageWSP image)
        : KisShapeLayerCanvasBase(parent, image)
        , m_layer(parent)
    {}

    void updateCanvas(const QRectF &rect) override
    {
        QRectF r = m_viewConverter->documentToView(rect);
        m_layer->signalUpdate(r);
    }

    void forceRepaint() override
    {
        m_layer->signalUpdate(m_layer->extent());
    }

    void setImage(KisImageWSP image) override {}
    void prepareForDestroying() override {}

private:
    KisReferenceImagesLayer *m_layer;
};
KisReferenceImagesLayer::KisReferenceImagesLayer(KoShapeBasedDocumentBase* shapeController, KisImageWSP image)
    : KisShapeLayer(shapeController, image, i18n("Reference images"), OPACITY_OPAQUE_U8, new ReferenceImagesCanvas(this, image))
{}

KUndo2Command * KisReferenceImagesLayer::addReferenceImage(KisReferenceImage *referenceImage)
{
    return new AddReferenceImageCommand(this, referenceImage);
}

void KisReferenceImagesLayer::paint(QPainter &painter) {
    shapeManager()->paint(painter, *converter(), false);
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

void KisReferenceImagesLayer::signalUpdate(const QRectF &rect)
{
    emit sigUpdateCanvas(rect);
}

QColor KisReferenceImagesLayer::getPixel(QPointF position) const
{
    const QPointF docPoint = converter()->viewToDocument(position);

    KoShape *shape = shapeManager()->shapeAt(docPoint);

    if (shape) {
        auto *reference = dynamic_cast<KisReferenceImage*>(shape);
        KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(reference, false);

        return reference->getPixel(docPoint);
    }

    return QColor();
}
