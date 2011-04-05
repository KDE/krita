/* This file is part of the KDE project
 * Copyright (C) 2010 Thomas Zander <zander@kde.org>
 * Copyright (C) 2010 Sebastian Sauer <sebsauer@kdab.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "ShrinkToFitShapeContainer.h"

#include <KoShapeSavingContext.h>

ShrinkToFitShapeContainer::ShrinkToFitShapeContainer(KoShape *childShape, KoResourceManager *documentResources)
    : KoShapeContainer(*(new ShrinkToFitShapeContainerPrivate(this, childShape)))
{
    Q_UNUSED(documentResources);
    Q_D(ShrinkToFitShapeContainer);

    setPosition(childShape->position());
    setSize(childShape->size());
    setZIndex(childShape->zIndex());
    rotate(childShape->rotation());
    //setTransformation(childShape->transformation());

    if (childShape->parent()) {
        childShape->parent()->addShape(this);
        childShape->setParent(0);
    }

    childShape->setPosition(QPointF(0.0,0.0)); // since its relative to my position, this won't move it
    childShape->setSelectable(false); // our ShrinkToFitShapeContainer will handle that from now on

    d->model = new ShrinkToFitShapeContainerModel(this, d);
    addShape(childShape);

    QSet<KoShape*> delegates;
    delegates << childShape;
    setToolDelegates(delegates);
    
    KoTextShapeData* data = dynamic_cast<KoTextShapeData*>(childShape->userData());
    Q_ASSERT(data);
    KoTextDocumentLayout *lay = qobject_cast<KoTextDocumentLayout*>(data->document()->documentLayout());
    Q_ASSERT(lay);
    QObject::connect(lay, SIGNAL(finishedLayout()), static_cast<ShrinkToFitShapeContainerModel*>(d->model), SLOT(finishedLayout()));
}

ShrinkToFitShapeContainer::~ShrinkToFitShapeContainer()
{
}

void ShrinkToFitShapeContainer::paintComponent(QPainter &painter, const KoViewConverter &converter)
{
    Q_UNUSED(painter);
    Q_UNUSED(converter);
    //painter.fillRect(converter.documentToView(QRectF(QPointF(0,0),size())), QBrush(QColor("#ffcccc"))); // for testing
}

bool ShrinkToFitShapeContainer::loadOdf(const KoXmlElement &element, KoShapeLoadingContext &context)
{
    Q_UNUSED(element);
    Q_UNUSED(context);
    return true;
}

void ShrinkToFitShapeContainer::saveOdf(KoShapeSavingContext &context) const
{
    Q_UNUSED(context);
}

ShrinkToFitShapeContainer* ShrinkToFitShapeContainer::wrapShape(KoShape *shape, KoResourceManager *documentResourceManager)
{
    Q_ASSERT(dynamic_cast<KoTextShapeData*>(shape->userData()));
    Q_ASSERT(qobject_cast<KoTextDocumentLayout*>(dynamic_cast<KoTextShapeData*>(shape->userData())->document()->documentLayout()));

    return new ShrinkToFitShapeContainer(shape, documentResourceManager);
}

void ShrinkToFitShapeContainer::tryWrapShape(KoShape *shape, const KoXmlElement &element, KoShapeLoadingContext &context)
{
    KoTextShapeData* data = dynamic_cast<KoTextShapeData*>(shape->userData());
    KoTextDocumentLayout *lay = qobject_cast<KoTextDocumentLayout*>(data ? data->document()->documentLayout() : 0);
    if (!lay || lay->resizeMethod() != KoTextDocumentLayout::ShrinkToFitResize)
        return;

    KoShapeContainer *oldParent = shape->parent();
    ShrinkToFitShapeContainer *tos = wrapShape(shape, context.documentResourceManager());
    if (!tos->loadOdf(element, context)) {
        shape->setParent(oldParent);
        delete tos;
    }
}

void ShrinkToFitShapeContainer::unwrapShape(KoShape *shape)
{
    Q_ASSERT(shape->parent() == this);

    removeShape(shape);
    shape->setParent(parent());

    QSet<KoShape*> delegates = toolDelegates();
    delegates.remove(shape);
    setToolDelegates(delegates);

    shape->setPosition(position());
    shape->setSize(size());
    shape->rotate(rotation());
    shape->setSelectable(true);
}

ShrinkToFitShapeContainerModel::ShrinkToFitShapeContainerModel(ShrinkToFitShapeContainer *q, ShrinkToFitShapeContainerPrivate *d)
    : q(q)
    , d(d)
    , m_scaleX(1.0)
    , m_scaleY(1.0)
    , m_dirty(10)
    , m_maybeUpdate(false)
{
}

void ShrinkToFitShapeContainerModel::finishedLayout()
{
    m_maybeUpdate = true;
    containerChanged(q, KoShape::SizeChanged);
    m_maybeUpdate = false;
}

void ShrinkToFitShapeContainerModel::containerChanged(KoShapeContainer *container, KoShape::ChangeType type)
{
    Q_ASSERT(container == q);
    if (type == KoShape::SizeChanged) {
        KoTextShapeData* data = dynamic_cast<KoTextShapeData*>(d->childShape->userData());
        Q_ASSERT(data);
        KoTextDocumentLayout *lay = qobject_cast<KoTextDocumentLayout*>(data->document()->documentLayout());
        Q_ASSERT(lay);

        QSizeF shapeSize = q->size();
        QSizeF documentSize = lay->documentSize();

        if (m_maybeUpdate && shapeSize == m_shapeSize && documentSize == m_documentSize) {
            m_dirty = 0;
            return; // nothing to update
        }

        m_shapeSize = shapeSize;
        m_documentSize = documentSize;

        if (documentSize.width() > 0.0 && documentSize.height() > 0.0) {
            if (m_dirty || !m_maybeUpdate) {
                m_scaleX = qMin<qreal>(1.0, shapeSize.width() / documentSize.width());
                m_scaleY = qMin<qreal>(1.0, shapeSize.height() / documentSize.height());
                m_scaleX = m_scaleY = (m_scaleX+m_scaleY)/2.0 * 0.95;

                //FIXME WARNING TODO HACK: m_dirty is an int that starts with 10 cause we need to get the initial init done
                //but the textshape doesn't provide us a nice way to do so. So, what we got are a bunch of finishedLayout
                //calls and one of them (usually the last one in a row of such calls) is the correct one where the
                //layouting was REALLY done and not pseudo-done. We are interested in exactly that first initial call what
                //is the reason we are just handling all of the first 10 finishedLayout calls cause in all tested cases
                //we always got the expected final finishedLayout call during the first 10 of such calls.
                //The correct way would be to fix the textshape to not fire up a bunch of finishedLayout calls but to only
                //do so once everything is done.
                //Another problem this is working around is that calling d->childShape->setSize can in fact result in
                //another delayed finishedLayout call even if nothing changed (neither the size not the content nor...). In
                //some cases this was then leading to an infinite loop which was not 100% reprodcuable. So, by introducing
                //such a counter we try to guard us from that too.
                if (m_maybeUpdate && m_dirty)
                    --m_dirty;
            }
        } else { // seems layouting wasn't done yet or there is nothing we can shrink
            m_scaleX = m_scaleY = 1.0;
            m_dirty = 1; // nothing to do yet means that we still need to do something later if e.g. layouting was done
        }

        d->childShape->setSize(QSizeF(shapeSize.width() / m_scaleX, shapeSize.height() / m_scaleY));

        QTransform m;
        m.scale(m_scaleX, m_scaleY);
        d->childShape->setTransformation(m);
    }
}

bool ShrinkToFitShapeContainerModel::inheritsTransform(const KoShape *child) const
{
    Q_ASSERT(child == d->childShape);
    return true;
}

bool ShrinkToFitShapeContainerModel::isChildLocked(const KoShape *child) const
{
    Q_ASSERT(child == d->childShape);
    return true;
}

bool ShrinkToFitShapeContainerModel::isClipped(const KoShape *child) const
{
    Q_ASSERT(child == d->childShape);
    return false;
}
