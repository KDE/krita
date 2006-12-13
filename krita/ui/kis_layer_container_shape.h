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

#ifndef KIS_LAYER_COLLECTION_H_
#define KIS_LAYER_COLLECTION_H_

#include <QString>

#include <KoShapeContainer.h>
#include <kis_types.h>

class QPainter;
class KoViewConverter;


const QString KIS_LAYER_CONTAINER_ID = "KisLayerContainerShapeID";
/**
   The layer container is the flake shape that corresponds to
   KisGroupLayer. It contains any number of layers, including other
   group layers.

   XXX: Do we need to distinguish between LayerContainers and Layers?
   Can a LayerContainer, i.e. a KisGroupLayer have masks and
   selections, and if so, what does it mean? (BSAR)
 */
class KisLayerContainerShape : public KoShapeContainer
{

public:

    KisLayerContainerShape( KoShapeContainer * parent, KisLayerSP groupLayer );

    virtual ~KisLayerContainerShape();

public:

    KisLayerSP groupLayer();

    // KoShapeContainer implementation
    void paintComponent(QPainter &painter, const KoViewConverter &converter);

    // KoShape overrides
    bool isSelectable() const { return false; }

private:

    class Private;
    Private * m_d;
};

#endif
