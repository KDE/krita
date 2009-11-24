/*
 *  Copyright (c) 2006 Boudewijn Rempt <boud@valdyas.org>
 *            (c) 2009 Dmitry Kazakov <dimula73@gmail.com>
 *
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
#ifndef _KIS_MASK_
#define _KIS_MASK_

#include <QRect>

#include "kis_types.h"
#include "kis_node.h"
#include "kis_global.h"

#include <krita_export.h>

/**
 KisMask is the base class for all single channel
 mask-like paint devices in Krita. Masks can be rendered in different
 ways at different moments during the rendering stack. Masks are
 "owned" by layers (of any type), and cannot occur by themselves on
 themselves.

 The properties that masks implement are made available through the
 iterators created on their parent layer, or through iterators that
 can be created on the paint device that holds the mask data: masks
 are just paint devices, too.

 Masks should show up in the layerbox as sub-layers for the layer they
 are associated with and be ccp'able and draggable to other layers.

 Examples of masks are:

 - filter masks: like the alpha filter mask that is the most common
                 type of mask and is simply known as "mask" in the
                 gui. Other filter masks use any of krita's filters to
                 filter the pixels of their parent. (In this they
                 differ from adjustment layers, which filter all
                 layers under them in their group stack).

 - selections: the selection mask is rendered after composition and
   zooming and determines the selectedness of the pixels of the parent
   layer.

 - painterly overlays: painterly overlays indicate a particular
   property of the pixel in the parent paint device they are associated
   with, like wetness, height or gravity.

   XXX: For now, all masks are 8 bit. Make the channel depth settable.

 */
class KRITAIMAGE_EXPORT KisMask : public KisNode
{

    Q_OBJECT

public:

    /**
     * Create a new KisMask.
     */
    KisMask(const QString & name);

    /**
     * Copy the mask
     */
    KisMask(const KisMask& rhs);

    virtual ~KisMask();

    const KoColorSpace * colorSpace() const;
    const KoCompositeOp * compositeOp() const;

    /**
     * Return the selection associated with this mask. A selection can
     * contain both a paint device and shapes.
     */
    KisSelectionSP selection() const;

    /**
     * @return the selection: if you paint on mask, you paint on the selections
     */
    KisPaintDeviceSP paintDevice() const;

    /**
     * Change the selection to the specified selection object. The
     * selection is deep copied.
     */
    void setSelection(KisSelectionSP selection);

    /**
     * Selected the specified rect with the specified amount of selectedness.
     */
    void select(const QRect & rc, quint8 selectedness = MAX_SELECTED);

    /**
     * The extent and bounds of the mask are those of the selection inside
     */
    QRect extent() const;
    QRect exactBounds() const;

    /**
     * overridden from KisBaseNode
     */
    qint32 x() const;

    /**
     * overridden from KisBaseNode
     */
    void setX(qint32 x);

    /**
     * overridden from KisBaseNode
     */
    qint32 y() const;

    /**
     * overridden from KisBaseNode
     */
    void setY(qint32 y);

    virtual void setDirty();
    virtual void setDirty(const QRect & rect);
    virtual void setDirty(const QRegion & region);

    QRect needRect(const QRect &rect) const;
    QRect changeRect(const QRect &rect) const;
    QImage createThumbnail(qint32 w, qint32 h);

protected:
    /**
     * Apply the effect the projection using the mask as a selection.
     * Made public in KisEffectMask
     */
    virtual void apply(KisPaintDeviceSP projection, const QRect & rc) const;
    virtual QRect decorateRect(KisPaintDeviceSP &src,
                               KisPaintDeviceSP &dst,
                               const QRect & rc) const;

private:

    struct Private;

    Private * const m_d;

};


#endif
