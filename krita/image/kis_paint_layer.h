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

class QUndoCommand;

/**
 * This layer is of a type that can be painted on.
 */
class KRITAIMAGE_EXPORT KisPaintLayer : public KisLayer, public KisLayerSupportsIndirectPainting {
    typedef KisLayer super;

    Q_OBJECT

public:
    KisPaintLayer(KisImageWSP img, const QString& name, quint8 opacity, KisPaintDeviceSP dev);
    KisPaintLayer(KisImageWSP img, const QString& name, quint8 opacity);
    KisPaintLayer(KisImageWSP img, const QString& name, quint8 opacity, KoColorSpace * colorSpace);
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

    class Private;
    Private * m_d;
};

typedef KisSharedPtr<KisPaintLayer> KisPaintLayerSP;

#endif // KIS_PAINT_LAYER_H_

