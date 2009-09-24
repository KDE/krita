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

#include <ksharedptr.h>

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
    KisGroupLayer(KisImageWSP img, const QString &name, quint8 opacity);
    KisGroupLayer(const KisGroupLayer& rhs);
    virtual ~KisGroupLayer();

    KisNodeSP clone() const {
        return KisNodeSP(new KisGroupLayer(*this));
    }

    bool allowAsChild(KisNodeSP) const;

    QIcon icon() const;

    /// override from KisBaseNode
    void updateSettings();

    /**
     * XXX: make the colorspace of a layergroup user-settable: we want
     * to be able to have, for instance, a group of grayscale layers
     * resulting in a grayscale projection that is then merged with an
     * rgb image stack.
     */
    const KoColorSpace * colorSpace() const;
    KoColorSpace * colorSpace();

    void setColorSpace(const KoColorSpace* colorSpace, KoColorConversionTransformation::Intent renderingIntent = KoColorConversionTransformation::IntentPerceptual) const;

    /**
       Return the united extents of all layers in this group layer;
       this function is _recursive_.
     */
    QRect extent() const;

    /**
       Return the exact bounding rect of all layers in this group
       layer; this function is _recursive_ and can therefore be really
       slow.
     */
    QRect exactBounds() const;

    qint32 x() const;
    void setX(qint32 x);

    qint32 y() const;
    void setY(qint32 y);

    /**
       Accect the specified visitor.
       @return true if the operation succeeded, false if it failed.
    */
    bool accept(KisNodeVisitor &v);

    /**
       Clear the projection or create a projection from the specified
       paint divide.

       Warning: will copy from to, if !0,

       Note for hackers: implement CoW!
     */
    void resetProjection(KisPaintDeviceSP to = 0);

    /**
       Retrieve the projection for this group layer. Note that
       The projection is _not_ guaranteed to be up to date with
       the latest actions, and that you cannot discover whether it
       is!

       Note the second: this _may_ return the paint device of a paint
       layer if that paint layer is the only child of this group layer.
    */
    KisPaintDeviceSP projection() const;

    /**
     * @return 0 since there is no paint device associated with a group
     * layer.
     */
    KisPaintDeviceSP paintDevice() const;

    /**
     *  Update the given rect of the projection paint device.
     *
     * Note for hackers: keep this method thread-safe!
     */
    void updateProjection(const QRect & rc);

    QImage createThumbnail(qint32 w, qint32 h);

signals:

    void regionDirtied(const QRegion &);
    void rectDirtied(const QRect &);
    void settingsUpdated();

private:


    /// Returns if the layer will induce the projection hack (if the only layer in this group)
    bool paintLayerInducesProjectionOptimization(KisPaintLayerSP l) const;

    class Private;
    Private * const m_d;


};

#endif // KIS_GROUP_LAYER_H_

