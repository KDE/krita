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

#include "kis_node_shape.h"

#include <KoCanvasBase.h>
#include <KoCanvasController.h>
#include <KoViewConverter.h>
#include <KoSelection.h>
#include <KoShapeContainer.h>
#include <KoToolManager.h>
#include <KoShapeManager.h>

#include <kis_types.h>
#include <kis_layer.h>
#include <kis_node.h>
#include <kis_mask.h>
#include <kis_image.h>

#include <kis_paint_device.h>

struct KisNodeShape::Private
{
public:
    KisNodeSP node;
};

KisNodeShape::KisNodeShape(KisNodeSP node)
        : KoShapeLayer()
        , m_d(new Private())
{

    m_d->node = node;

    setShapeId(KIS_NODE_SHAPE_ID);

    setSelectable(false);

    connect(node, SIGNAL(visibilityChanged(bool)), SLOT(setNodeVisible(bool)));
    connect(node, SIGNAL(userLockingChanged(bool)), SLOT(editabilityChanged()));
    editabilityChanged();  // Correctly set the lock at loading
}

KisNodeShape::~KisNodeShape()
{
    if (KoToolManager::instance()) {
        KoCanvasController *canvasController = KoToolManager::instance()->activeCanvasController();
        // If we're the active layer, we should tell the active selection we're dead meat.
        if (canvasController && canvasController->canvas() && canvasController->canvas()->shapeManager()) {
            KoSelection *activeSelection = canvasController->canvas()->shapeManager()->selection();
            KoShapeLayer *activeLayer = activeSelection->activeLayer();
            if (activeLayer == this){
                activeSelection->setActiveLayer(0);
            }
        }
    }
    delete m_d;
}

KisNodeSP KisNodeShape::node()
{
    return m_d->node;
}

void KisNodeShape::setNodeVisible(bool /*v*/)
{
    // Necessary because shapes are not QObjects
//     setVisible(v);
}

bool KisNodeShape::checkIfDescendant(KoShapeLayer *activeLayer)
{
    bool found(false);
    KoShapeLayer *layer = activeLayer;

    while(layer && !(found = layer == this)) {
        layer = dynamic_cast<KoShapeLayer*>(layer->parent());
    }

    return found;
}

void KisNodeShape::editabilityChanged()
{
    if (m_d->node->inherits("KisShapeLayer")) {
        setGeometryProtected(!m_d->node->isEditable());
    } else {
        setGeometryProtected(false);
    }
    /**
     * Editability of a child depends on the editablity
     * of its parent. So when we change one's editability,
     * we need to search for active children and reactivate them
     */

    KoCanvasController *canvasController = KoToolManager::instance()->activeCanvasController();

    if(canvasController && canvasController->canvas() && canvasController->canvas()->shapeManager()) {
        KoSelection *activeSelection = canvasController->canvas()->shapeManager()->selection();
        KoShapeLayer *activeLayer = activeSelection->activeLayer();

        if(activeLayer && checkIfDescendant(activeLayer)) {
            activeSelection->setActiveLayer(activeLayer);
        }
    }

}

QSizeF KisNodeShape::size() const
{
    return boundingRect().size();
}

QRectF KisNodeShape::boundingRect() const
{
    return QRectF();
}

void KisNodeShape::setPosition(const QPointF &)
{
}

void KisNodeShape::paint(QPainter &, const KoViewConverter &, KoShapePaintingContext &)
{
}

void KisNodeShape::saveOdf(KoShapeSavingContext &) const
{
}

bool KisNodeShape::loadOdf(const KoXmlElement &, KoShapeLoadingContext &)
{
    return false;
}

