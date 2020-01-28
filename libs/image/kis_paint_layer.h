/*
 *  Copyright (c) 2005 C. Boemann <cbo@boemann.dk>
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
#ifndef KIS_PAINT_LAYER_H_
#define KIS_PAINT_LAYER_H_

#include "kis_types.h"
#include "kis_layer.h"
#include "kis_indirect_painting_support.h"
#include "KisDecoratedNodeInterface.h"

#include <QBitArray>

class KoColorSpace;

/**
 * This layer is of a type that can be drawn on. A paint layer can
 * have any number of effect masks, a transparency mask, a local
 * selection and a protection mask.
 *
 * The protection mask can be read/write, read-only or write-only.
 * The transparency mask has two rendering forms: as a selection mask
 * and by changing the transparency of the paint layer's pixels.
 */
class KRITAIMAGE_EXPORT KisPaintLayer : public KisLayer, public KisIndirectPaintingSupport, public KisDecoratedNodeInterface
{

    Q_OBJECT

public:
    /**
     * Construct a paint layer with the given parameters. The default bounds of the paintdevice are overwritten.
     * @param image this layer belongs to, or null, if it shouldn't belong to any image
     * @param name of the layer
     * @param opacity in the range between OPACITY_TRANSPARENT_U8 and OPACITY_OPAQUE_U8
     * @param dev is the paint device, that should be used
     */
    KisPaintLayer(KisImageWSP image, const QString& name, quint8 opacity, KisPaintDeviceSP dev);

    /**
     * Construct a paint layer with the given parameters
     * @param image this layer belongs to. it must not be null and it must have a valid color space.
     * @param name of the layer
     * @param opacity in the range between OPACITY_TRANSPARENT_U8 and OPACITY_OPAQUE_U8
     */
    KisPaintLayer(KisImageWSP image, const QString& name, quint8 opacity);

    /**
     * Construct a paint layer with the given parameters
     * @param image this layer belongs to, or null, if it shouldn't belong to any image. image must not be null, if colorSpace is null
     * @param name of the layer
     * @param opacity in the range between OPACITY_TRANSPARENT_U8 and OPACITY_OPAQUE_U8
     * @param colorSpace is the color space, that should be used to construct the paint device. it can be null, if the image is valid.
     */
    KisPaintLayer(KisImageWSP image, const QString& name, quint8 opacity, const KoColorSpace * colorSpace);
    /**
     * Copy Constructor
     */
    KisPaintLayer(const KisPaintLayer& rhs);
    ~KisPaintLayer() override;

    KisNodeSP clone() const override {
        return KisNodeSP(new KisPaintLayer(*this));
    }

    bool allowAsChild(KisNodeSP) const override;

    const KoColorSpace * colorSpace() const override;

    bool needProjection() const override;

    QIcon icon() const override;
    void setImage(KisImageWSP image) override;

    KisBaseNode::PropertyList sectionModelProperties() const override;
    void setSectionModelProperties(const KisBaseNode::PropertyList &properties) override;

public:

    QRect extent() const override;
    QRect exactBounds() const override;

    bool accept(KisNodeVisitor &v) override;
    void accept(KisProcessingVisitor &visitor, KisUndoAdapter *undoAdapter) override;

    /**
     * set the channelflags for locking certain channels (used by painting tools)
     * for this layer to the specified bit array.
     * The bit array must have exactly the same number of channels as
     * the colorspace this layer is in, or be empty, in which case all
     * channels are active.
     */
    void setChannelLockFlags(const QBitArray& channelFlags);
    
    /**
     * Return a bit array where each bit indicates whether a
     * particular channel is locked or not (used by painting tools).
     * If the channelflags bit array is empty, all channels are active.
     */
    const QBitArray& channelLockFlags() const;

    /**
     * Returns the paintDevice that accompanies this layer
     */
    KisPaintDeviceSP paintDevice() const override;

    /**
     * Returns the original pixels before masks have been applied.
     */
    KisPaintDeviceSP original() const override;

    /**
     * @returns true when painting should not affect the alpha channel
     */
    bool alphaLocked() const;

    /**
     * @param l if true, the alpha channel will be protected from modification
     */
    void setAlphaLocked(bool lock);

    /**
     * @return true if onion skins should be rendered on this layer
     */
    bool onionSkinEnabled() const;

    /**
     * @param state whether onion skins should be rendered
     */
    void setOnionSkinEnabled(bool state);

    KisPaintDeviceList getLodCapableDevices() const override;

    bool decorationsVisible() const override;
    void setDecorationsVisible(bool value, bool update) override;
    using KisDecoratedNodeInterface::setDecorationsVisible;

public Q_SLOTS:
    void slotExternalUpdateOnionSkins();


public:

    // KisIndirectPaintingSupport
    KisLayer* layer() {
        return this;
    }

protected:
    // override from KisLayer
    void copyOriginalToProjection(const KisPaintDeviceSP original,
                                  KisPaintDeviceSP projection,
                                  const QRect& rect) const override;

    KisKeyframeChannel *requestKeyframeChannel(const QString &id) override;

private:
    void init(KisPaintDeviceSP paintDevice, const QBitArray &paintChannelFlags = QBitArray());

    struct Private;
    Private * const m_d;
};

typedef KisSharedPtr<KisPaintLayer> KisPaintLayerSP;

#endif // KIS_PAINT_LAYER_H_

