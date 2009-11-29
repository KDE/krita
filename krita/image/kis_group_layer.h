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
#ifndef KIS_GROUP_LAYER_H_
#define KIS_GROUP_LAYER_H_

#include "kis_layer.h"
#include "kis_types.h"
#include <KoColorConversionTransformation.h>

class KoColorSpace;

/**
 * A KisLayer that bundles child layers into a single layer.
 * The top layer is firstChild(), with index 0; the bottommost lastChild() with index childCount() - 1.
 * KisLayer::nextSibling() moves towards higher indices, from the top to the bottom layer; prevSibling() the reverse.
 * (Implementation detail: internally, the indices are reversed, for speed.)
 **/
class KRITAIMAGE_EXPORT KisGroupLayer : public KisLayer
{
    Q_OBJECT

public:
    KisGroupLayer(KisImageWSP image, const QString &name, quint8 opacity);
    KisGroupLayer(const KisGroupLayer& rhs);
    virtual ~KisGroupLayer();

    KisNodeSP clone() const {
        return KisNodeSP(new KisGroupLayer(*this));
    }

    bool allowAsChild(KisNodeSP) const;

    QIcon icon() const;

    /**
     * Clear the projection
     */
    void resetCache(const KoColorSpace *colorSpace = 0);

    /**
     * XXX: make the colorspace of a layergroup user-settable: we want
     * to be able to have, for instance, a group of grayscale layers
     * resulting in a grayscale projection that is then merged with an
     * rgb image stack.
     */
    const KoColorSpace * colorSpace() const;

    /// @return the projection of the layers in the group before the masks are applied.
    KisPaintDeviceSP original() const;

    /**
     * If a dirty node is set, repaintOriginal can optimize by checking
     * whether there is an adjustment layer in the stack, under the dirty
     * node.
     *
     * XXX: protect by a mutex?
     */
    void setDirtyNode(KisNodeSP node);

    QRect repaintOriginal(KisPaintDeviceSP original, const QRect& rect);

    qint32 x() const;
    qint32 y() const;
    void setX(qint32 x);
    void setY(qint32 y);

    /**
       Accect the specified visitor.
       @return true if the operation succeeded, false if it failed.
    */
    bool accept(KisNodeVisitor &v);

protected:
    KisPaintDeviceSP tryObligeChild() const;

private:
    class Private;
    Private * const m_d;
};

#endif // KIS_GROUP_LAYER_H_

