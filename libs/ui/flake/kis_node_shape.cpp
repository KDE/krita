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
#include <KoSelection.h>
#include <KoToolManager.h>

#include <kis_types.h>
#include <kis_layer.h>
#include <kis_node.h>

#include <KoSelectedShapesProxy.h>
#include "kis_shape_layer.h"


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

    connect(node, SIGNAL(sigNodeChangedInternal()), SLOT(editabilityChanged()));
    editabilityChanged();  // Correctly set the lock at loading
}

KisNodeShape::~KisNodeShape()
{
    if (KoToolManager::instance()) {
        KoCanvasController *canvasController = KoToolManager::instance()->activeCanvasController();
        // If we're the active layer, we should tell the active selection we're dead meat.
        if (canvasController && canvasController->canvas()) {
            KoSelection *activeSelection = canvasController->canvas()->selectedShapesProxy()->selection();
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

    Q_FOREACH (KoShape *shape, this->shapes()) {
        KisNodeShape *node = dynamic_cast<KisNodeShape*>(shape);
        KIS_SAFE_ASSERT_RECOVER(node) { continue; }
        if (node) {
            node->editabilityChanged();
        }
    }

    /**
     * Editability of a child depends on the editablity
     * of its parent. So when we change one's editability,
     * we need to search for active children and reactivate them
     */

    KoCanvasController *canvasController = KoToolManager::instance()->activeCanvasController();

    if(canvasController && canvasController->canvas()) {
        KoSelection *activeSelection = canvasController->canvas()->selectedShapesProxy()->selection();
        KoShapeLayer *activeLayer = activeSelection->activeLayer();


        KisShapeLayer *shapeLayer = dynamic_cast<KisShapeLayer*>(m_d->node.data());

        if(activeLayer && (checkIfDescendant(activeLayer) || (shapeLayer && shapeLayer == activeLayer))) {
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

void KisNodeShape::paint(QPainter &, KoShapePaintingContext &)
{
}

void KisNodeShape::saveOdf(KoShapeSavingContext &) const
{
}

bool KisNodeShape::loadOdf(const KoXmlElement &, KoShapeLoadingContext &)
{
    return false;
}

