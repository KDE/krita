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

class KisNodeShape::Private
{
public:
    KisNodeSP node;
};

KisNodeShape::KisNodeShape(KoShapeContainer * parent, KisNodeSP node)
        : KoShapeLayer()
        , m_d(new Private())
{

    m_d->node = node;

    setShapeId(KIS_NODE_SHAPE_ID);
    KoShape::setParent(parent);
    parent->addChild(this);

    connect(node, SIGNAL(visibilityChanged(bool)), SLOT(setNodeVisible(bool)));
    connect(node, SIGNAL(userLockingChanged(bool)), SLOT(editabilityChanged()));
    connect(node, SIGNAL(systemLockingChanged(bool)), SLOT(editabilityChanged()));
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

void KisNodeShape::paint(QPainter &painter, const KoViewConverter &converter)
{
    Q_UNUSED(painter);
    Q_UNUSED(converter);
}


void KisNodeShape::paintComponent(QPainter &painter, const KoViewConverter &converter)
{
    Q_UNUSED(painter);
    Q_UNUSED(converter);
}

QSizeF KisNodeShape::size() const
{
    Q_ASSERT(m_d);
    Q_ASSERT(m_d->node);

    QRect br = m_d->node->extent();

    KisImageWSP image = getImage();

    if (!image) return QSizeF(0.0, 0.0);

    dbgUI << "KisNodeShape::size extent:" << br << ", x res:" << image->xRes() << ", y res:" << image->yRes();

    return QSizeF(br.width() / image->xRes(), br.height() / image->yRes());
}

QRectF KisNodeShape::boundingRect() const
{
    QRect br = m_d->node->extent();

    KisImageWSP image = getImage();

    return QRectF(int(br.left()) / image->xRes(), int(br.top()) / image->yRes(),
                  int(1 + br.right()) / image->xRes(), int(1 + br.bottom()) / image->yRes());

}

void KisNodeShape::setPosition(const QPointF & position)
{
    Q_ASSERT(m_d);
    Q_ASSERT(m_d->node);

    KisImageWSP image = getImage();

    if (image) {
        // XXX: Does flake handle undo for us?
        QPointF pf(position.x() / image->xRes(), position.y() / image->yRes());
        QPoint p = pf.toPoint();
        m_d->node->setX(p.x());
        m_d->node->setY(p.y());
    }
}


void KisNodeShape::addChild(KoShape * shape)
{
    KisNodeShape* nodeShape = dynamic_cast<KisNodeShape*>(shape);
    if (!nodeShape) return;

    if (!m_d->node->allowAsChild(nodeShape->node())) return;

    KoShapeContainer::addChild(shape);
}

void KisNodeShape::saveOdf(KoShapeSavingContext & /*context*/) const
{

    // TODO
}

bool KisNodeShape::loadOdf(const KoXmlElement & /*element*/, KoShapeLoadingContext &/*context*/)
{
    return false; // TODO
}

void KisNodeShape::setNodeVisible(bool v)
{
    // Necessary because shapes are not QObjects
    setVisible(v);
}

// Defined in KisNodeContainerShape... FIXME (2.1) find a better way to share code between those two classes, or even better merge them
bool recursiveFindActiveLayerInChildren(KoSelection* _selection, KoShapeLayer* _currentLayer);

void KisNodeShape::editabilityChanged()
{
    dbgKrita << m_d->node->isEditable();
    setGeometryProtected(!m_d->node->isEditable());
    KoCanvasController* canvas = KoToolManager::instance()->activeCanvasController();
    if (canvas) {
        recursiveFindActiveLayerInChildren(canvas->canvas()->shapeManager()->selection(), this);
    }
}

KisImageWSP KisNodeShape::getImage() const
{

    if (m_d->node->inherits("KisLayer")) {
        return dynamic_cast<KisLayer*>(m_d->node.data())->image();
    } else if (m_d->node->inherits("KisMask")) {
        return dynamic_cast<KisLayer*>(m_d->node->parent().data())->image();
    }

    return 0;
}

#include "kis_node_shape.moc"
