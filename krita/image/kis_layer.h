/*
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
 *  Copyright (c) 2005 Casper Boemann <cbr@boemann.dk>
 *  Copyright (c) 2007 Boudewijn Rempt <boud@valdyas.org>
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

#include "KoCompositeOp.h"
#include "KoDocumentSectionModel.h"

#include "kis_types.h"
#include "kis_node.h"
#include "kis_paint_device.h"

class QIcon;
class QBitArray;
class KisGroupLayer;
class KoColorSpace;
class KisNodeVisitor;

namespace KisMetaData {
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

    KisLayer(KisImageWSP img, const QString &name, quint8 opacity);
    KisLayer(const KisLayer& rhs);
    virtual ~KisLayer();

    virtual KoColorSpace * colorSpace();

    /**
     * Ask the layer to assemble its data & apply all the effect masks
     * to it.
     */
    virtual void updateProjection(const QRect& r) = 0;

    /**
     * Return the fully rendered representation of this layer: its
     * data and its effect masks
     */
    virtual KisPaintDeviceSP projection() const = 0;

    /**
     * Return the layer data before the effect masks have had their go
     * at it.
     */
    virtual KisPaintDeviceSP original() const;

    /**
     * Return the paintdevice you can use to change pixels on. For a
     * paint layer these will be paint pixels, for an adjustment layer
     * the selection paint device.
     *
     * @return the paint device to paint on. Can be 0 if the actual
     * layer type does not support painting.
     */
    virtual KisPaintDeviceSP paintDevice() const = 0;

    /**
     * @return the selection associated with this layer, if there is
     * one. Otherwise, return 0;
     */
    virtual KisSelectionMaskSP selectionMask() const;

    virtual KoDocumentSectionModel::PropertyList sectionModelProperties() const;
    virtual void setSectionModelProperties( const KoDocumentSectionModel::PropertyList &properties  );

    /**
     * set the channelflags for this layer to the specified bit array.
     * The bit array must have exactly the same number of channels as
     * the colorspace this layer is in, or be empty, in which case all
     * channels are active.
     */
    void setChannelFlags( const QBitArray & channelFlags );

    /**
     * Return a bit array where each bit indicates whether a
     * particular channel is active or not. If the channelflags bit
     * array is empty, all channels are active.
     */
    QBitArray & channelFlags();


    /**
     * Return the opacity of this layer, scaled to a range between 0
     * and 255.
     * XXX: Allow true float opacity
     */
    quint8 opacity() const; //0-255

    /**
     * Set the opacity for this layer. The range is between 0 and 255.
     * The layer will be marked dirty.
     *
     * XXX: Allow true float opacity
     */
    void setOpacity(quint8 val); //0-255

    /**
     * return the 8-bit opacity of this layer scaled to the range
     * 0-100
     *
     * XXX: Allow true float opacity
     */
    quint8 percentOpacity() const; //0-100

    /**
     * Set the opacity of this layer with a number between 0 and 100;
     * the number will be scaled to between 0 and 255.
     * XXX: Allow true float opacity
     */
    void setPercentOpacity(quint8 val); //0-100

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

    /**
     * Return the composite op associated with this layer.
     */
    const KoCompositeOp * compositeOp() const;

    /**
     * Set a new composite op for this layer. The layer will be marked
     * dirty.
     */
    void setCompositeOp(const KoCompositeOp * compositeOp);


    KisImageSP image() const;

    /**
     * Set the image this layer belongs to.
     */
    void setImage(KisImageSP image);

public:

    /**
     * Returns true if there are any effect masks present
     */
    bool hasEffectMasks() const;

    /**
     * @return the list of effect masks
     */
    QList<KisMaskSP> effectMasks() const;

    /**
     * Set a temporary effect mask on this layer for filter previews.
     */
    void setPreviewMask( KisEffectMaskSP mask );

    /**
     * Retrieve the current preview effect mask
     */
    KisEffectMaskSP previewMask() const;

    KisLayerSP KDE_DEPRECATED parentLayer() const;

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
     * Apply the effect masks to the given projection, producing
     * finally the dst paint device.
     */
    void applyEffectMasks( KisPaintDeviceSP projection, const QRect & rc );

private:
    class Private;
    Private * const m_d;

};

/**
 * For classes that support indirect painting.
 *
 * XXX: Name doesn't suggest an object -- is KisIndirectPaintingLayer
 * a better name? (BSAR)
 */
class KRITAIMAGE_EXPORT KisIndirectPaintingSupport {

public:

    KisIndirectPaintingSupport();
    virtual ~KisIndirectPaintingSupport();

    // Indirect painting
    void setTemporaryTarget(KisPaintDeviceSP t);
    void setTemporaryCompositeOp(const KoCompositeOp* c);
    void setTemporaryOpacity(quint8 o);
    KisPaintDeviceSP temporaryTarget();
    const KoCompositeOp* temporaryCompositeOp() const;
    quint8 temporaryOpacity() const;

    // Or I could make KisLayer a virtual base of KisIndirectPaintingSupport and so, but
    // I'm sure virtual diamond inheritance isn't as appreciated as this

    virtual KisLayer* layer() = 0;

private:

    // To simulate the indirect painting
    KisPaintDeviceSP m_temporaryTarget;
    const KoCompositeOp* m_compositeOp;
    quint8 m_compositeOpacity;

};

Q_DECLARE_METATYPE( KisLayerSP )

#endif // KIS_LAYER_H_
