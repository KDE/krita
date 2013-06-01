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

#include "KoDocumentSectionModel.h"

#include "kis_types.h"
#include "kis_node.h"

template <class T>
class QStack;

class QIcon;
class QBitArray;
class KisCloneLayer;
class KisNodeVisitor;

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

    /**
     * Ask the layer to assemble its data & apply all the effect masks
     * to it.
     */
    virtual QRect updateProjection(const QRect& rect);

    virtual bool needProjection() const;

    virtual void copyOriginalToProjection(const KisPaintDeviceSP original,
                                          KisPaintDeviceSP projection,
                                          const QRect& rect) const;

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

    virtual KoDocumentSectionModel::PropertyList sectionModelProperties() const;
    virtual void setSectionModelProperties(const KoDocumentSectionModel::PropertyList &properties);

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
    QList<KisEffectMaskSP> effectMasks() const;

    QRect changeRect(const QRect &rect, PositionToFilthy pos = N_FILTHY) const;

    /**
     * Set a temporary effect mask on this layer for filter previews.
     */
    void setPreviewMask(KisEffectMaskSP mask);

    /**
     * Retrieve the current preview effect mask
     */
    KisEffectMaskSP previewMask() const;

    /**
     * Get the group layer that contains this layer.
     */
    KisLayerSP parentLayer() const;

    /**
     * Remove the temporary effect mask.
     */
    void removePreviewMask();

    /**
     * @return the metadata object associated with this object.
     */
    KisMetaData::Store* metaData();

protected:

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
                     const QRect &requestedRect) const;

private:
    struct Private;
    Private * const m_d;
};

Q_DECLARE_METATYPE(KisLayerSP)

#endif // KIS_LAYER_H_
