/*
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
#ifndef KIS_PAINT_LAYER_H_
#define KIS_PAINT_LAYER_H_

#include "kis_types.h"
#include "kis_layer.h"
#include "kis_paint_device.h"

class KoColorSpace;

const QString KIS_PAINT_LAYER_ID = "KisPaintLayer";
/**
 * This layer is of a type that can be d on. A paint layer can
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

    virtual QString nodeType()
        {
            return KIS_PAINT_LAYER_ID;
        }

    virtual bool canHaveChildren()
        {
            return true;
        }


    KoColorSpace * colorSpace();

    void updateProjection(const QRect& r);
    KisPaintDeviceSP projection() const;

    QIcon icon() const;
    KoDocumentSectionModel::PropertyList properties() const;
    KisLayerSP clone() const;

public:

    qint32 x() const;
    void setX(qint32 x);

    qint32 y() const;
    void setY(qint32 y);

    QRect extent() const;
    QRect exactBounds() const;

    QImage createThumbnail(qint32 w, qint32 h);

    bool accept(KisLayerVisitor &v);
    /**
     * Returns the paintDevice that accompanies this layer
     */
    KisPaintDeviceSP paintDevice() const;

    /**
     * Returns the original pixels before masks have been applied.
     * This is the same as the paintDevice() OVER driedPaintDevice();
     */
    KisPaintDeviceSP original() const;

    /**
     * Return the dried pixels -- i.e., pixels that have been fixed
     * through the physics process. This may be 0. The physics process
     * will automatically set this paint device when it becomes needed.
     */
    KisPaintDeviceSP driedPaintDevice();

    /**
     * Sets the dried paint device associated with this paint layer to
     * 0.
     */
    void removeDriedPaintDevice();

    /**
     * Return the painterly overlay -- a special paint device that
     * uses the KisPainterlyOverlayColorSpace that defines things
     * like wetness and canvas height. You need to explicitly create
     * the painterly overlay before accessing it. Running a physics
     * filter may also create the painterly overlay.
     *
     * Note: this may be 0.
     */
    KisPainterlyOverlaySP painterlyOverlay();

    /**
     * Create a painterly overlay on this paint layer
     */
    void createPainterlyOverlay();

    /**
     * Sets the painterly overlay associated with this paint layer to 0.
     */
    void removePainterlyOverlay();

public slots:

    /// Overridden to call the private convertMaskToSelection
    void setDirty();
    void setDirty(const QRect & rect);
    void setDirty(const QRegion & region);

    // KisIndirectPaintingSupport
    KisLayer* layer() { return this; }

private slots:
    void slotColorSpaceChanged();

private:
    void init();

    class Private;
    Private * const m_d;
};

typedef KisSharedPtr<KisPaintLayer> KisPaintLayerSP;

#endif // KIS_PAINT_LAYER_H_

