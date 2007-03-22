/*
 *  Copyright (c) 2005 Casper Boemann <cbr@boemann.dk>
 *  Copyright (c) 2007 Boudewijn Rempt <boud@valdyas.org>
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

class KoColorSpace;
class QUndoCommand;

/**
 * This layer is of a type that can be painted on. A paint layer can
 * have any number of effect masks, a transparency mask, a local
 * selection and a protection mask.
 *
 * The protection mask can be read/write, read-only or write-only.
 * The transparency mask has two rendering forms: as a selection mask
 * and by changing the transparency of the paint layer's pixels.
 */
class KRITAIMAGE_EXPORT KisPaintLayer : public KisLayer, public KisIndirectPaintingSupport {
    typedef KisLayer super;

    Q_OBJECT

public:
    KisPaintLayer(KisImageSP img, const QString& name, quint8 opacity, KisPaintDeviceSP dev);
    KisPaintLayer(KisImageSP img, const QString& name, quint8 opacity);
    KisPaintLayer(KisImageSP img, const QString& name, quint8 opacity, KoColorSpace * colorSpace);
    KisPaintLayer(const KisPaintLayer& rhs);
    virtual ~KisPaintLayer();

    KoColorSpace * colorSpace();

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

    virtual void paint(QImage &img, qint32 x, qint32 y, qint32 w, qint32 h);
    virtual void paint(QImage &img, const QRect& scaledImageRect, const QSize& scaledImageSize, const QSize& imageSize);

    virtual void paintMaskInactiveLayers(QImage &img, qint32 x, qint32 y, qint32 w, qint32 h);

    virtual QImage createThumbnail(qint32 w, qint32 h);

    virtual bool accept(KisLayerVisitor &v)
        {
            return v.visit(this);
        }


    /// Returns the paintDevice that accompanies this layer
    KisPaintDeviceSP paintDevice() const;

    /// Returns the paintDevice that accompanies this layer (or mask, see editMask)
    KisPaintDeviceSP paintDeviceOrMask() const;

    // Mask Layer

    /// Does this layer have a layer mask?
    bool hasMask() const;

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
    QUndoCommand* createMaskCommand();
    /// Undoable version of createMaskFromSelection
    QUndoCommand* maskFromSelectionCommand();
    /// Undoable, removes the current mask, but converts it to the current selection
    QUndoCommand* maskToSelectionCommand();
    /// Undoable version of removeMask
    QUndoCommand* removeMaskCommand();
    /// Undoable version of applyMask
    QUndoCommand* applyMaskCommand();

    /// Returns true if the masked part of the mask will be rendered instead of being transparent
    bool renderMask() const;
    /// Set the renderMask property
    void setRenderMask(bool b);

    /**
     * When this returns true, the KisPaintDevice returned in paintDevice will actually
     * be the layer mask (if there is one). This is so that tools can draw on the mask
     * without needing to know its existence.
     */
    bool editMask() const;

    /// Sets the editMask property
    void setEditMask(bool b);

    /// Overridden to call the private convertMaskToSelection
    virtual void setDirty();
    virtual void setDirty(const QRect & rect);
    virtual void setDirty(const QRegion & region);

    // KisIndirectPaintingSupport
    virtual KisLayer* layer() { return this; }

    /**
       Clear the projection or create a projection from the specified
       paint devide.

       Warning: will copy from to, if !0,

       Note for hackers: implement CoW!
     */
    virtual void resetProjection(KisPaintDeviceSP to = 0);

    /**
       Retrieve the projection for this group layer. Note that
       The projection is _not_ guaranteed to be up to date with
       the latest actions, and that you cannot discover whether it
       is!

       Note the second: this _may_ return the paint device of a paint
       layer if that paint layer is the only child of this group layer.
    */
    virtual KisPaintDeviceSP projection();

    /**
       Update the given rect of the projection paint device.

       Note for hackers: keep this method thread-safe!
    */
    void updateProjection(const QRect & rc);


signals:
    /// When the mask is created/destroyed or the editmask or rendermask is changed
    void sigMaskInfoChanged();

private slots:
    void slotColorSpaceChanged();

private:
    void init();
    void convertMaskToSelection(const QRect& r);
    void genericMaskCreationHelper();

    class Private;
    Private * m_d;
};

typedef KisSharedPtr<KisPaintLayer> KisPaintLayerSP;

#endif // KIS_PAINT_LAYER_H_

