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
class KRITAIMAGE_EXPORT KisPaintLayer : public KisLayer, public KisIndirectPaintingSupport
{

    Q_OBJECT

public:
    KisPaintLayer(KisImageWSP img, const QString& name, quint8 opacity, KisPaintDeviceSP dev);
    KisPaintLayer(KisImageWSP img, const QString& name, quint8 opacity);
    KisPaintLayer(KisImageWSP img, const QString& name, quint8 opacity, const KoColorSpace * colorSpace);
    KisPaintLayer(const KisPaintLayer& rhs);
    virtual ~KisPaintLayer();

    KisNodeSP clone() const {
        return KisNodeSP(new KisPaintLayer(*this));
    }

    bool allowAsChild(KisNodeSP) const;

    const KoColorSpace * colorSpace() const;

    QRect repaintOriginal(KisPaintDeviceSP original,
                          const QRect& rect);

    bool needProjection() const;

    void copyOriginalToProjection(const KisPaintDeviceSP original,
                                  KisPaintDeviceSP projection,
                                  const QRect& rect) const;

    QIcon icon() const;

    KoDocumentSectionModel::PropertyList sectionModelProperties() const;
    void setSectionModelProperties(const KoDocumentSectionModel::PropertyList &properties);

public:

    QRect extent() const;
    QRect exactBounds() const;

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
     * @returns true when painting should not affect the alpha channel
     */
    bool alphaLocked() const;

    /**
     * @param l if true, the alpha channel will be protected from modification
     */
    void setAlphaLocked(bool l);

public slots:

    // KisIndirectPaintingSupport
    KisLayer* layer() {
        return this;
    }


private:

    class Private;
    Private * const m_d;
};

typedef KisSharedPtr<KisPaintLayer> KisPaintLayerSP;

#endif // KIS_PAINT_LAYER_H_

