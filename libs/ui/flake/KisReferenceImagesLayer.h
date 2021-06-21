/*
 * SPDX-FileCopyrightText: 2017 Jouni Pentikäinen <joupent@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KRITA_KISREFERENCEIMAGESLAYER_H
#define KRITA_KISREFERENCEIMAGESLAYER_H

#include "kis_shape_layer.h"

#include <kis_types.h>
#include <KisView.h>

class KisDocument;
class KoCanvasBase;

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

    KUndo2Command* setProfile(const KoColorProfile *profile) override;
    KUndo2Command* convertTo(const KoColorSpace * dstColorSpace,
                                 KoColorConversionTransformation::Intent renderingIntent = KoColorConversionTransformation::internalRenderingIntent(),
                                 KoColorConversionTransformation::ConversionFlags conversionFlags = KoColorConversionTransformation::internalConversionFlags()) override;


    bool getLock();
    void setLock(bool, KoCanvasBase*);
    QTransform getLockedFlakeToWidgetTransform();
    QTransform getLockedDocToViewTransform();

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

    bool lock;
    QTransform lockedFlakeToWidgetTransform;
    QTransform lockedDocToViewTransform;
};

typedef KisSharedPtr<KisReferenceImagesLayer> KisReferenceImagesLayerSP;


#endif //KRITA_KISREFERENCEIMAGESLAYER_H
