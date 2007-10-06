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

    Q_OBJECT

public:
    KisPaintLayer(KisImageSP img, const QString& name, quint8 opacity, KisPaintDeviceSP dev);
    KisPaintLayer(KisImageSP img, const QString& name, quint8 opacity);
    KisPaintLayer(KisImageSP img, const QString& name, quint8 opacity, KoColorSpace * colorSpace);
    KisPaintLayer(const KisPaintLayer& rhs);
    virtual ~KisPaintLayer();

    KisNodeSP clone()
        {
            return KisNodeSP(new KisPaintLayer(*this));
        }

    bool allowAsChild( KisNodeSP );

    KoColorSpace * colorSpace();

    /**
     * Update the projection for the specified rect r, whether that rect is dirty
     * or not.
     */
    void updateProjection(const QRect& r);

    /**
     * Return the projection paint device, or 0 if the projection does
     * not exist yet. (For instance, because it hasn't been updated yet.)
     */
    KisPaintDeviceSP projection() const;

    QIcon icon() const;
    KoDocumentSectionModel::PropertyList sectionModelProperties() const;
    KisLayerSP clone() const;

public:

    qint32 x() const;
    void setX(qint32 x);

    qint32 y() const;
    void setY(qint32 y);

    QRect extent() const;
    QRect exactBounds() const;

    QImage createThumbnail(qint32 w, qint32 h);

    bool accept(KisNodeVisitor &v);
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

public slots:

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

