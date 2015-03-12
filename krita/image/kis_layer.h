/*
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
 *  Copyright (c) 2005 C. Boemann <cbo@boemann.dk>
 *  Copyright (c) 2007 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2009 Dmitry Kazakov <dimula73@gmail.com>
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
#ifndef KIS_LAYER_H_
#define KIS_LAYER_H_

#include <QRect>
#include <QRegion>
#include <QMetaType>
#include <QObject>

#include "krita_export.h"

#include "KisDocumentSectionModel.h"

#include "kis_types.h"
#include "kis_node.h"

template <class T>
class QStack;

class QBitArray;
class KisCloneLayer;
class KisPSDLayerStyle;
class KisAbstractProjectionPlane;


namespace KisMetaData
{
class Store;
}

/**
 * Abstract class that represents the concept of a Layer in Krita. This is not related
 * to the paint devices: this is merely an abstraction of how layers can be stacked and
 * rendered differently.
 * Regarding the previous-, first-, next- and lastChild() calls, first means that it the layer
 * is at the top of the group in the layerlist, using next will iterate to the bottom to last,
 * whereas previous will go up to first again.
 *
 *
 * TODO: Add a layer mode whereby the projection of the layer is used
 * as a clipping path?
 **/
class KRITAIMAGE_EXPORT KisLayer : public KisNode
{

    Q_OBJECT

public:

    /**
     * @param image is the pointer of the image or null
     * @param opacity is a value between OPACITY_TRANSPARENT_U8 and OPACITY_OPAQUE_U8
    **/
    KisLayer(KisImageWSP image, const QString &name, quint8 opacity);
    KisLayer(const KisLayer& rhs);
    virtual ~KisLayer();

    /// returns the image's colorSpace or null, if there is no image
    virtual const KoColorSpace * colorSpace() const;

    /// returns the layer's composite op for the colorspace of the layer's parent.
    const KoCompositeOp * compositeOp() const;

    KisPSDLayerStyle *layerStyle() const;
    void setLayerStyle(KisPSDLayerStyle *layerStyle);

    /**
     * \see a comment in KisNode::projectionPlane()
     */
    virtual KisAbstractProjectionPlane* projectionPlane() const;

    /**
     * The projection plane representing the layer itself without any
     * styles or anything else. It is used by the layer styles projection
     * plane to stack up the planes.
     */
    virtual KisAbstractProjectionPlane* internalProjectionPlane() const;

    QRect partialChangeRect(KisNodeSP lastNode, const QRect& rect);
    void buildProjectionUpToNode(KisPaintDeviceSP projection, KisNodeSP lastNode, const QRect& rect);

    virtual bool needProjection() const;

    /**
     * Return the fully rendered representation of this layer: its
     * data and its effect masks
     */
    virtual KisPaintDeviceSP projection() const;

    /**
     * Return the layer data before the effect masks have had their go
     * at it.
     */
    virtual KisPaintDeviceSP original() const = 0;

    /**
     * @return the selection associated with this layer, if there is
     * one. Otherwise, return 0;
     */
    virtual KisSelectionMaskSP selectionMask() const;

    /**
     * @return the selection contained in the first KisSelectionMask associated
     * with this layer or the image, if either exists, otherwise, return 0.
     */
    virtual KisSelectionSP selection() const;

    virtual KisDocumentSectionModel::PropertyList sectionModelProperties() const;
    virtual void setSectionModelProperties(const KisDocumentSectionModel::PropertyList &properties);

    /**
     * set/unset the channel flag for the alpha channel of this layer
     */
    void disableAlphaChannel(bool disable);

    /**
     * returns true if the channel flag for the alpha channel
     * of this layer is not set.
     * returns false otherwise.
     */
    bool alphaChannelDisabled() const;

    /**
     * set the channelflags for this layer to the specified bit array.
     * The bit array must have exactly the same number of channels as
     * the colorspace this layer is in, or be empty, in which case all
     * channels are active.
     */
    virtual void setChannelFlags(const QBitArray & channelFlags);

    /**
     * Return a bit array where each bit indicates whether a
     * particular channel is active or not. If the channelflags bit
     * array is empty, all channels are active.
     */
    QBitArray & channelFlags() const;

    /**
     * Returns true if this layer is temporary: i.e., it should not
     * appear in the layerbox, even though it is temporarily in the
     * layer stack and taken into account on recomposition.
     */
    bool temporary() const;

    /**
     * Set to true if this layer should not appear in the layerbox,
     * even though it is temporarily in the layer stack and taken into
     * account on recomposition.
     */
    void setTemporary(bool t);

    /// returns the image this layer belongs to, or null if there is no image
    KisImageWSP image() const;

    /**
     * Set the image this layer belongs to.
     */
    virtual void setImage(KisImageWSP image);

    /**
     * Create and return a layer that is the result of merging
     * this with layer.
     *
     * This method is designed to be called only within KisImage::mergeLayerDown().
     *
     * Decendands override this to create specific merged types when possible.
     * The KisLayer one creates a KisPaintLayerSP via a bitBlt, and can work on all layer types.
     *
     * Decendants that perform there own version do NOT call KisLayer::createMergedLayer
     */
    virtual KisLayerSP createMergedLayer(KisLayerSP prevLayer);

    /**
     * Clones should be informed about updates of the original
     * layer, so this is a way to register them
     */
    void registerClone(KisCloneLayerWSP clone);

    /**
     * Deregisters the clone from the update list
     *
     * \see registerClone()
     */
    void unregisterClone(KisCloneLayerWSP clone);

    /**
     * Return the list of the clones of this node. Be careful
     * with the list, because it is not thread safe.
     */
    const QList<KisCloneLayerWSP> registeredClones() const;


    /**
     * Returns whether we have a clone.
     *
     * Be careful with it. It is not thread safe to add/remove
     * clone while checking hasClones(). So there should be no updates.
     */
    bool hasClones() const;

    /**
     * It is calles by the async merger after projection update is done
     */
    void updateClones(const QRect &rect);

public:
    qint32 x() const;
    qint32 y() const;

    void setX(qint32 x);
    void setY(qint32 y);

    /**
     * Returns an approximation of where the bounds
     * of actual data of this layer are
     */
    QRect extent() const;

    /**
     * Returns the exact bounds of where the actual data
     * of this layer resides
     */
    QRect exactBounds() const;

    QImage createThumbnail(qint32 w, qint32 h);

public:
    /**
     * Returns true if there are any effect masks present
     */
    bool hasEffectMasks() const;

    /**
     * @return the list of effect masks
     */
    QList<KisEffectMaskSP> effectMasks(KisNodeSP lastNode = 0) const;

    /**
     * Get the group layer that contains this layer.
     */
    KisLayerSP parentLayer() const;

    /**
     * @return the metadata object associated with this object.
     */
    KisMetaData::Store* metaData();

protected:
    // override from KisNode
    QRect changeRect(const QRect &rect, PositionToFilthy pos = N_FILTHY) const;

protected:

    /**
     * Ask the layer to assemble its data & apply all the effect masks
     * to it.
     */
    QRect updateProjection(const QRect& rect, KisNodeSP filthyNode);

    /**
     * Layers can override this method to get some special behavior
     * when copying data from \p original to \p projection, e.g. blend
     * in indirect painting device.  If you need to modify data
     * outside \p rect, please also override outgoingChangeRect()
     * method.
     */
    virtual void copyOriginalToProjection(const KisPaintDeviceSP original,
                                          KisPaintDeviceSP projection,
                                          const QRect& rect) const;
    /**
     * For KisLayer classes change rect transformation consists of two
     * parts: incoming and outgoing.
     *
     * 1) incomingChangeRect(rect) chande rect transformation
     *    performed by the transformations done basing on global
     *    projection. It is performed in KisAsyncMerger +
     *    KisUpdateOriginalVisitor classes. It happens before data
     *    coming to KisLayer::original() therefore it is
     *    'incoming'. See KisAdjustmentLayer for example of usage.
     *
     * 2) outgoingChangeRect(rect) change rect transformation that
     *    happens in KisLayer::copyOriginalToProjection(). It applies
     *    *only* when the layer is 'filthy', that is was the cause of
     *    the merge process. See KisCloneLayer for example of usage.
     *
     * The flow of changed areas can be illustrated in the
     * following way:
     *
     * 1. Current projection of size R1 is stored in KisAsyncMerger::m_currentProjection
     *      |
     *      | <-- KisUpdateOriginalVisitor writes data into layer's original() device.
     *      |     The changed area on KisLayer::original() is
     *      |     R2 = KisLayer::incomingChangeRect(R1)
     *      |
     * 2. KisLayer::original() / changed rect: R2
     *      |
     *      | <-- KisLayer::updateProjection() starts composing a layer
     *      |     It calls KisLayer::copyOriginalToProjection() which copies some area
     *      |     to a temporaty device. The temporary device now stores
     *      |     R3 = KisLayer::outgoingChangeRect(R2)
     *      |
     * 3. Temporary device / changed rect: R3
     *      |
     *      | <-- KisLayer::updateProjection() continues composing a layer. It merges a mask.
     *      |     R4 = KisMask::changeRect(R3)
     *      |
     * 4. KisLayer::original() / changed rect: R4
     *
     * So in the end rect R4 will be passed up to the next layers in the stack.
     */
    virtual QRect incomingChangeRect(const QRect &rect) const;

    /**
     * \see incomingChangeRect()
     */
    virtual QRect outgoingChangeRect(const QRect &rect) const;

    /**
     * @param rectVariesFlag (out param) a flag, showing whether
     *        a rect varies from mask to mask
     * @return an area that should be updated because of
     *         the change of @requestedRect of the layer
     */
    QRect masksChangeRect(const QList<KisEffectMaskSP> &masks,
                          const QRect &requestedRect,
                          bool &rectVariesFlag) const;

    /**
     * Get needRects for all masks
     * @param changeRect requested rect to be updated on final
     *        projection. Should be a return value
     *        of @ref masksChangedRect()
     * @param applyRects (out param) a stack of the rects where filters
     *        should be applied
     * @param rectVariesFlag (out param) a flag, showing whether
     *        a rect varies from mask to mask
     * @return a needRect that should be prepared on the layer's
     *         paintDevice for all masks to succeed
     */
    QRect masksNeedRect(const QList<KisEffectMaskSP> &masks,
                        const QRect &changeRect,
                        QStack<QRect> &applyRects,
                        bool &rectVariesFlag) const;

    QRect applyMasks(const KisPaintDeviceSP source,
                     const KisPaintDeviceSP destination,
                     const QRect &requestedRect,
                     KisNodeSP filthyNode, KisNodeSP lastNode) const;
private:
    friend class KisLayerProjectionPlane;
    friend class KisTransformMask;
    friend class KisLayerTest;

private:
    struct Private;
    Private * const m_d;
};

Q_DECLARE_METATYPE(KisLayerSP)

#endif // KIS_LAYER_H_
