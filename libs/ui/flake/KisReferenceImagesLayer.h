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

class KisDocument;

class KRITAUI_EXPORT KisReferenceImagesLayer : public KisShapeLayer
{
    Q_OBJECT

public:
    KisReferenceImagesLayer(KoShapeControllerBase* shapeController, KisImageWSP image);
    KisReferenceImagesLayer(const KisReferenceImagesLayer &rhs);

    static KUndo2Command * addReferenceImages(KisDocument *document, QList<KoShape*> referenceImages);
    KUndo2Command * removeReferenceImages(KisDocument *document, QList<KoShape*> referenceImages);
    QVector<KisReferenceImage*> referenceImages() const;

    QRectF boundingImageRect() const;
    QColor getPixel(QPointF position) const;

    void paintReferences(QPainter &painter);

    bool allowAsChild(KisNodeSP) const override;

    bool accept(KisNodeVisitor&) override;
    void accept(KisProcessingVisitor &visitor, KisUndoAdapter *undoAdapter) override;

    KisNodeSP clone() const override {
        return new KisReferenceImagesLayer(*this);
    }

    bool isFakeNode() const override;

Q_SIGNALS:
    /**
     * The content of the layer has changed, and the canvas decoration
     * needs to update.
     */
    void sigUpdateCanvas(const QRectF &rect);

private:
    void signalUpdate(const QRectF &rect);
    friend struct AddReferenceImagesCommand;
    friend struct RemoveReferenceImagesCommand;
    friend class ReferenceImagesCanvas;
};


#endif //KRITA_KISREFERENCEIMAGESLAYER_H
