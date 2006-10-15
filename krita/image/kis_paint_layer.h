/*
 *  Copyright (c) 2005 Casper Boemann <cbr@boemann.dk>
 *
 *  this program is free software; you can redistribute it and/or modify
 *  it under the terms of the gnu general public license as published by
 *  the free software foundation; either version 2 of the license, or
 *  (at your option) any later version.
 *
 *  this program is distributed in the hope that it will be useful,
 *  but without any warranty; without even the implied warranty of
 *  merchantability or fitness for a particular purpose.  see the
 *  gnu general public license for more details.
 *
 *  you should have received a copy of the gnu general public license
 *  along with this program; if not, write to the free software
 *  foundation, inc., 675 mass ave, cambridge, ma 02139, usa.
 */
#ifndef KIS_PAINT_LAYER_H_
#define KIS_PAINT_LAYER_H_

#include "kis_types.h"
#include "kis_layer.h"
#include "kis_paint_device.h"
#include "KoColorSpace.h"
/**
 * This layer is of a type that can be painted on.
 */
class KRITAIMAGE_EXPORT KisPaintLayer : public KisLayer, public KisLayerSupportsIndirectPainting {
    typedef KisLayer super;

    Q_OBJECT

public:
    KisPaintLayer(KisImage *img, const QString& name, quint8 opacity, KisPaintDeviceSP dev);
    KisPaintLayer(KisImage *img, const QString& name, quint8 opacity);
    KisPaintLayer(KisImage *img, const QString& name, quint8 opacity, KoColorSpace * colorSpace);
    KisPaintLayer(const KisPaintLayer& rhs);
    virtual ~KisPaintLayer();

    virtual QIcon icon() const;
    virtual PropertyList properties() const;
    virtual KisLayerSP clone() const;
public:

    virtual qint32 x() const;
    virtual void setX(qint32 x);

    virtual qint32 y() const;
    virtual void setY(qint32 y);

    virtual QRect extent() const;
    virtual QRect exactBounds() const;

    virtual void paintSelection(QImage &img, qint32 x, qint32 y, qint32 w, qint32 h);
    virtual void paintSelection(QImage &img, const QRect& scaledImageRect, const QSize& scaledImageSize, const QSize& imageSize);

    virtual void paintMaskInactiveLayers(QImage &img, qint32 x, qint32 y, qint32 w, qint32 h);

    virtual QImage createThumbnail(qint32 w, qint32 h);

    virtual bool accept(KisLayerVisitor &v)
        {
//            kDebug(41001) << "\tPAINT\t" << name()
//                    << " dirty: " << dirty() << "\n";
            return v.visit(this);
        }


    /// Returns the paintDevice that accompanies this layer
    inline KisPaintDeviceSP paintDevice() const { return m_paintdev; }

    /// Returns the paintDevice that accompanies this layer (or mask, see editMask)
    inline KisPaintDeviceSP paintDeviceOrMask() const {
        if (hasMask() && editMask())
            return m_mask;
        return m_paintdev;
    }

    // Mask Layer

    /// Does this layer have a layer mask?
    bool hasMask() const { return !m_mask.isNull(); }
    // XXX TODO: Make these undo-able!
    /// Create a mask if it does not yet exist, and return it
    KisPaintDeviceSP createMask();
    /// Convert the from argument to the mask
    void createMaskFromPaintDevice(KisPaintDeviceSP from);
    /**
     * Convert the from selection to a paint device (should convert the getMaskAsSelection
     * result back to the mask). Overwrites the current mask, if any. Also removes the selection
     */
    void createMaskFromSelection(KisSelectionSP from);
    /// Remove the layer mask
    void removeMask();
    /// Apply the layer mask to the paint device, this removes the mask afterwards
    void applyMask();
    /// Returns the layer mask's device. Creates one if there is currently none
    KisPaintDeviceSP getMask();
    /// Returns the layer mask's device, converted to a selection. Creates one if there is currently none
    KisSelectionSP getMaskAsSelection();

    /// Undoable version of createMask
    KNamedCommand* createMaskCommand();
    /// Undoable version of createMaskFromSelection
    KNamedCommand* maskFromSelectionCommand();
    /// Undoable, removes the current mask, but converts it to the current selection
    KNamedCommand* maskToSelectionCommand();
    /// Undoable version of removeMask
    KNamedCommand* removeMaskCommand();
    /// Undoable version of applyMask
    KNamedCommand* applyMaskCommand();

    /// Returns true if the masked part of the mask will be rendered instead of being transparent
    bool renderMask() const { return m_renderMask; }
    /// Set the renderMask property
    void setRenderMask(bool b);

    /**
     * When this returns true, the KisPaintDevice returned in paintDevice will actually
     * be the layer mask (if there is one). This is so that tools can draw on the mask
     * without needing to know its existence.
     */
    bool editMask() const { return m_editMask; }
    /// Sets the editMask property
    void setEditMask(bool b);

    /// Overridden to call the private convertMaskToSelection
    virtual void setDirty(bool propagate = true);
    /// Same as above
    virtual void setDirty(const QRect & rect, bool propagate = true);

    // KisLayerSupportsIndirectPainting
    virtual KisLayer* layer() { return this; }

signals:
    /// When the mask is created/destroyed or the editmask or rendermask is changed
    void sigMaskInfoChanged();

private slots:
    void slotColorSpaceChanged();

private:
    void init();
    void convertMaskToSelection(const QRect& r);
    void genericMaskCreationHelper();

    KisPaintDeviceSP m_paintdev;
    // Layer mask related:
    // XXX It would be nice to merge the next 2 devices...
    KisPaintDeviceSP m_mask; // The mask that we can edit and display easily
    KisSelectionSP m_maskAsSelection; // The mask as selection, to apply and render easily
    bool m_renderMask;
    bool m_editMask;
};

typedef KisSharedPtr<KisPaintLayer> KisPaintLayerSP;

#endif // KIS_PAINT_LAYER_H_

