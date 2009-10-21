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

#include "kis_layer_container_shape.h"


#include <QPainter>

#include <KoCanvasBase.h>
#include <KoCanvasController.h>
#include <KoViewConverter.h>
#include <KoSelection.h>
#include <KoShapeContainer.h>
#include <KoToolManager.h>
#include <KoShapeManager.h>

#include <kis_types.h>
#include <kis_group_layer.h>
#include <kis_image.h>

class KisLayerContainerShape::Private
{

public:
    KisLayerSP groupLayer;
};

KisLayerContainerShape::KisLayerContainerShape(KoShapeContainer *parent, KisLayerSP groupLayer)
        : KoShapeLayer()
        , m_d(new Private)
{
    m_d->groupLayer = groupLayer;

    KoShape::setParent(parent);
    setShapeId(KIS_LAYER_CONTAINER_ID);

    connect(groupLayer, SIGNAL(visibilityChanged(bool)), SLOT(setLayerVisible(bool)));
    connect(groupLayer, SIGNAL(userLockingChanged(bool)), SLOT(editabilityChanged()));
    connect(groupLayer, SIGNAL(systemLockingChanged(bool)), SLOT(editabilityChanged()));
}

KisLayerContainerShape::~KisLayerContainerShape()
{
    delete m_d;
}

KisLayerSP KisLayerContainerShape::groupLayer()
{
    return m_d->groupLayer;
}


QSizeF KisLayerContainerShape::size() const
{
    KisImageWSP image = m_d->groupLayer->image();
    if (!image) return QSize(0, 0);

    QSize br = image->size();
    return QSizeF(br.width() / image->xRes(), br.height() / image->yRes());
}

QRectF KisLayerContainerShape::boundingRect() const
{
    KisImageWSP image = m_d->groupLayer->image();
    if (!image) return QRect();

    QRect br = QRect(0, 0, image->size().width(), image->size().height());
    return QRectF(int(br.left()) / image->xRes(), int(br.top()) / image->yRes(),
                  int(1 + br.right()) / image->xRes(), int(1 + br.bottom()) / image->yRes());

}

void KisLayerContainerShape::paintComponent(QPainter &painter, const KoViewConverter &converter)
{
    Q_UNUSED(painter);
    Q_UNUSED(converter);
}

void KisLayerContainerShape::saveOdf(KoShapeSavingContext & /*context*/) const
{
    // TODO
}

bool KisLayerContainerShape::loadOdf(const KoXmlElement & /*element*/, KoShapeLoadingContext &/*context*/)
{
    return false; // TODO
}

void KisLayerContainerShape::setLayerVisible(bool v)
{
    setVisible(v);
}

bool recursiveFindActiveLayerInChildren(KoSelection* _selection, KoShapeLayer* _currentLayer)
{
    if (_selection->activeLayer() == _currentLayer) {
        // The following looks weird but it is needed to update the status of the tools
        _selection->setActiveLayer(_currentLayer);
        return true;
    }
    foreach(KoShape* shape, _currentLayer->childShapes()) {
        KoShapeLayer* layer = dynamic_cast<KoShapeLayer*>(shape);
        if (layer && recursiveFindActiveLayerInChildren(_selection, layer)) {
            return true;
        }
    }
    return false;
}

void KisLayerContainerShape::editabilityChanged()
{
    dbgKrita << m_d->groupLayer->isEditable();
    setGeometryProtected(!m_d->groupLayer->isEditable());
    KoCanvasController* canvas = KoToolManager::instance()->activeCanvasController();
    if (canvas) {
        recursiveFindActiveLayerInChildren(canvas->canvas()->shapeManager()->selection(), this);
    }
}

#include "kis_layer_container_shape.moc"
