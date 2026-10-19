/*
 *  SPDX-FileCopyrightText: 2006 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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

void KisNodeShape::paint(QPainter &) const
{
}
