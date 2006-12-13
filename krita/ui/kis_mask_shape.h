/*
 *  Copyright (c) 2006 Boudewijn Rempt <boud@valdyas.org>
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

#ifndef KIS_MASK_SHAPE_H_
#define KIS_MASK_SHAPE_H_

#include <KoShape.h>

#include <kis_types.h>

class QPainter;

class KoViewConverter;
class KoShapeContainer;

#define KIS_MASK_SHAPE_ID "KisMaskShapeID"

/**
   A KisMaskShape is a flake shape wrapper around a KisMask. A KisMask
   is a single-channel (for now) 8-bit (for now) mask that belongs to
   a layer. Examples of masks are alpha masks (that hide part of a
   layer), filter masks (that filter part of a layer), selection masks
   (that determine which part of a layer is selected), painterly masks
   (that add some aspect of real-world media to the colors represented
   by a layer) or channel masks (that contain a single color channel
   extracted from the actual layer).
*/

class KisMaskShape : public KoShape
{
public:

    KisMaskShape( KoShapeContainer * parent, KisMaskSP mask );
    ~KisMaskShape();

    void paint(QPainter &painter, const KoViewConverter &converter)
        {
            Q_UNUSED( painter );
            Q_UNUSED( converter );
            // Do nothing! Masks don't paint on QPainters
        }
    bool isSelectable() const { return false; }


private:

    class Private;
    Private * m_d;
};

#endif //KIS_MASK_SHAPE_H_
