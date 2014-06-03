/* This file is part of the KDE project

   Copyright (C) 2006-2008 Thorsten Zachmann <zachmann@kde.org>
   Copyright (C) 2006-2010 Thomas Zander <zander@kde.org>
   Copyright (C) 2009-2010 Jan Hambrecht <jaham@gmx.net>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/
#ifndef KoShapeManager_p_h
#define KoShapeManager_p_h

#include "KoShapeManager.h"
#include "KoSelection.h"
#include "KoToolManager.h"
#include "KoPointerEvent.h"
#include "KoShape.h"
#include "KoShape_p.h"
#include "KoCanvasBase.h"
#include "KoShapeContainer.h"
#include "KoShapeStrokeModel.h"
#include "KoShapeGroup.h"
#include "KoToolProxy.h"
#include "KoShapeManagerPaintingStrategy.h"
#include "KoShapeShadow.h"
#include "KoShapeLayer.h"
#include "KoFilterEffect.h"
#include "KoFilterEffectStack.h"
#include "KoFilterEffectRenderContext.h"
#include "KoShapeBackground.h"
#include <KoRTree.h>
#include "KoClipPath.h"
#include "KoShapePaintingContext.h"

#include <QPainter>
#include <QTimer>
#include <kdebug.h>

class KoShapeManager::Private
{
public:
    Private(KoShapeManager *shapeManager, KoCanvasBase *c)
        : selection(new KoSelection()),
          canvas(c),
          tree(4, 2),
          strategy(new KoShapeManagerPaintingStrategy(shapeManager)),
          q(shapeManager)
    {
    }

    ~Private() {
        delete selection;
        delete strategy;
    }

    /**
     * Update the tree when there are shapes in m_aggregate4update. This is done so not all
     * updates to the tree are done when they are asked for but when they are needed.
     */
    void updateTree();

    /**
     * Recursively paints the given group shape to the specified painter
     * This is needed for filter effects on group shapes where the filter effect
     * applies to all the children of the group shape at once
     */
    void paintGroup(KoShapeGroup *group, QPainter &painter, const KoViewConverter &converter, KoShapePaintingContext &paintContext);

    class DetectCollision
    {
    public:
        DetectCollision() {}
        void detect(KoRTree<KoShape *> &tree, KoShape *s, int prevZIndex) {
            foreach(KoShape *shape, tree.intersects(s->boundingRect())) {
                bool isChild = false;
                KoShapeContainer *parent = s->parent();
                while (parent && !isChild) {
                    if (parent == shape)
                        isChild = true;
                    parent = parent->parent();
                }
                if (isChild)
                    continue;
                if (s->zIndex() <= shape->zIndex() && prevZIndex <= shape->zIndex())
                    // Moving a shape will only make it collide with shapes below it.
                    continue;
                if (shape->collisionDetection() && !shapesWithCollisionDetection.contains(shape))
                    shapesWithCollisionDetection.append(shape);
            }
        }

        void fireSignals() {
            foreach(KoShape *shape, shapesWithCollisionDetection)
                shape->priv()->shapeChanged(KoShape::CollisionDetected);
        }

    private:
        QList<KoShape*> shapesWithCollisionDetection;
    };

    QList<KoShape *> shapes;
    QList<KoShape *> additionalShapes; // these are shapes that are only handled for updates
    KoSelection *selection;
    KoCanvasBase *canvas;
    KoRTree<KoShape *> tree;
    QSet<KoShape *> aggregate4update;
    QHash<KoShape*, int> shapeIndexesBeforeUpdate;
    KoShapeManagerPaintingStrategy *strategy;
    KoShapeManager *q;
};

#endif