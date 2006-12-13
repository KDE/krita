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

#ifndef KIS_LAYER_SHAPE_H_
#define KIS_LAYER_SHAPE_H_

#include <KoShapeContainer.h>
#include <KoShape.h>
#include <KoViewConverter.h>

#include <kis_types.h>

#define KIS_LAYER_SHAPE_ID "KisLayerShape"

/**
   A KisLayerShape is a flake wrapper around adjustment layers or paint
   layers. A layershape can only have a KisMaskShape as its descendant.
 */
class KisLayerShape : public KoShapeContainer
{

public:

    KisLayerShape( KoShapeContainer * parent, KisLayerSP layer );
    virtual ~KisLayerShape();

    KisLayerSP layer();

    // Shape overrides
    void paint(QPainter &painter, const KoViewConverter &converter);
    bool isSelectable() const { return false; }

    // KoShapeContainer implementation
    void paintComponent(QPainter &painter, const KoViewConverter &converter);

    void addChild( KoShape * shape );

private:

    class Private;
    Private * m_d;
};

#endif
