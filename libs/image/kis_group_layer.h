/*
 *  SPDX-FileCopyrightText: 2005 C. Boemann <cbo@boemann.dk>
 *  SPDX-FileCopyrightText: 2007 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KIS_GROUP_LAYER_H_
#define KIS_GROUP_LAYER_H_

#include "kis_layer.h"
#include "kis_types.h"

class KoColorSpace;

/**
 * A KisLayer that bundles child layers into a single layer.
 * The top layer is firstChild(), with index 0; the bottommost lastChild() with index childCount() - 1.
 * KisLayer::nextSibling() moves towards higher indices, from the top to the bottom layer; prevSibling() the reverse.
 * (Implementation detail: internally, the indices are reversed, for speed.)
 **/
class KRITAIMAGE_EXPORT KisGroupLayer : public KisLayer
{
    Q_OBJECT

public:
    KisGroupLayer(KisImageWSP image, const QString &name, quint8 opacity);
    KisGroupLayer(const KisGroupLayer& rhs);
    ~KisGroupLayer() override;

    KisNodeSP clone() const override {
        return KisNodeSP(new KisGroupLayer(*this));
    }

    bool allowAsChild(KisNodeSP) const override;

    QIcon icon() const override;

    KisBaseNode::PropertyList sectionModelProperties() const override;
    void setSectionModelProperties(const KisBaseNode::PropertyList &properties) override;

    void setImage(KisImageWSP image) override;

    KisLayerSP createMergedLayerTemplate(KisLayerSP prevLayer) override;
    void fillMergedLayerTemplate(KisLayerSP dstLayer, KisLayerSP prevLayer) override;

    /**
     * Clear the projection
     */
    void resetCache(const KoColorSpace *colorSpace = 0);

    /**
     * XXX: make the colorspace of a layergroup user-settable: we want
     * to be able to have, for instance, a group of grayscale layers
     * resulting in a grayscale projection that is then merged with an
     * rgb image stack.
     */
    const KoColorSpace * colorSpace() const override;

    /// @return the projection of the layers in the group before the masks are applied.
    KisPaintDeviceSP original() const override;

    qint32 x() const override;
    qint32 y() const override;
    void setX(qint32 x) override;
    void setY(qint32 y) override;

    /// Group layers don't have a paint device, so return 0
    KisPaintDeviceSP paintDevice() const override;

    /**
       Accept the specified visitor.
       @return true if the operation succeeded, false if it failed.
    */
    bool accept(KisNodeVisitor &v) override;
    void accept(KisProcessingVisitor &visitor, KisUndoAdapter *undoAdapter) override;

    /**
     * A special method that changes the default color of the
     * projection merged onto this group layer. Please note, that you
     * cannot use original()->setDefaultPixel(), because original()
     * device can be switched by tryOblidgeChild() mechanism randomly.
     */
    void setDefaultProjectionColor(KoColor color);

    /**
     * \see setDefaultProjectionColor()
     */
    KoColor defaultProjectionColor() const;

    bool passThroughMode() const;
    void setPassThroughMode(bool value);

    QRect extent() const override;
    QRect exactBounds() const override;

    bool projectionIsValid() const;

protected:
    KisLayer* onlyMeaningfulChild() const;
    KisPaintDeviceSP tryObligeChild() const;

    QRect amortizedProjectionRectForCleanupInChangePass() const override;
private:
    bool checkCloneLayer(KisCloneLayerSP clone) const;
    bool checkNodeRecursively(KisNodeSP node) const;

private:
    struct Private;
    Private * const m_d;
};

#endif // KIS_GROUP_LAYER_H_

