/*
 *  Copyright (c) 2006 Boudewijn Rempt <boud@valdyas.org>
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
#include "kis_paint_device.h"

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

 - painterly mask: painterly masks indicate a particular property of the
   pixel in the parent paint device they are associated with, like
   wetness, height or gravity.

 - channels: a layer can be decomposed into masks that have one color
   channel.

   XXX: For now, all masks are 8 bit. Make the channel depth settable.

 */


enum EnumKisMaskRenderMoment {
    MergeWithLayerBeforeComposition,  ///< The mask will be painted before the parent is composited with in the layer stack. Eg: traditional masks or filter masks.
    RenderAfterLayerComposition, ///< The mask will be painted onto
                                 ///the layer stack right after the parent is merged
                                 ///with the layers below it.
    RenderAfterMerge, ///< The mask will painted on top of the finished projection before scaling,
    RenderOnTop ///< the mask will be painted on top of the finished projection after scaling. Example: selection.
};

class KRITAIMAGE_EXPORT KisMask : public KisPaintDevice {

public:

    KisMask(KisPaintDeviceSP dev, const QString & name);

    /**
     * Create a new KisMask. This selection will not have a parent paint device.
     */
    KisMask(const QString & name);

    /**
     * Copy the mask
     */
    KisMask(const KisMask& rhs);

    virtual ~KisMask();

    KisPaintDeviceSP parent();

    bool active();

    EnumKisMaskRenderMoment renderWhen();

private:

    class KisMaskPrivate;

    KisMaskPrivate * m_d;

};


#endif
