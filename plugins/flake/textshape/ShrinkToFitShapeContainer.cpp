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
#include <KoTextLayoutRootArea.h>

ShrinkToFitShapeContainer::ShrinkToFitShapeContainer(KoShape *childShape, KoDocumentResourceManager *documentResources)
    : KoShapeContainer()
    , d(new Private(childShape))
{
    Q_UNUSED(documentResources);

    setPosition(childShape->position());
    setSize(childShape->size());
    setZIndex(childShape->zIndex());
    setRunThrough(childShape->runThrough());
    rotate(childShape->rotation());
    //setTransformation(childShape->transformation());

    if (childShape->parent()) {
        childShape->parent()->addShape(this);
        childShape->setParent(0);
    }

    childShape->setPosition(QPointF(0.0, 0.0)); // since its relative to my position, this won't move it
    childShape->setSelectable(false); // our ShrinkToFitShapeContainer will handle that from now on

    setModel(new ShrinkToFitShapeContainerModel(this));
    addShape(childShape);

    QSet<KoShape *> delegates;
    delegates << childShape;
    setToolDelegates(delegates);

    KoTextShapeData *data = dynamic_cast<KoTextShapeData *>(childShape->userData());
    Q_ASSERT(data);
    KoTextDocumentLayout *lay = qobject_cast<KoTextDocumentLayout *>(data->document()->documentLayout());
    Q_ASSERT(lay);
    QObject::connect(lay, SIGNAL(finishedLayout()), static_cast<ShrinkToFitShapeContainerModel *>(model()), SLOT(finishedLayout()));
}

ShrinkToFitShapeContainer::~ShrinkToFitShapeContainer()
{
}

void ShrinkToFitShapeContainer::paintComponent(QPainter &painter, KoShapePaintingContext &)
{
    Q_UNUSED(painter);
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
    d->childShape->saveOdf(context);
}

ShrinkToFitShapeContainer *ShrinkToFitShapeContainer::wrapShape(KoShape *shape, KoDocumentResourceManager *documentResourceManager)
{
    Q_ASSERT(dynamic_cast<KoTextShapeData *>(shape->userData()));
    Q_ASSERT(qobject_cast<KoTextDocumentLayout *>(dynamic_cast<KoTextShapeData *>(shape->userData())->document()->documentLayout()));

    return new ShrinkToFitShapeContainer(shape, documentResourceManager);
}

void ShrinkToFitShapeContainer::tryWrapShape(KoShape *shape, const KoXmlElement &element, KoShapeLoadingContext &context)
{
    KoTextShapeData *data = dynamic_cast<KoTextShapeData *>(shape->userData());
    if (!data || data->resizeMethod() != KoTextShapeData::ShrinkToFitResize) {
        return;
    }

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

    QSet<KoShape *> delegates = toolDelegates();
    delegates.remove(shape);
    setToolDelegates(delegates);

    shape->setPosition(position());
    shape->setSize(size());
    shape->rotate(rotation());
    shape->setSelectable(true);
}

ShrinkToFitShapeContainerModel::ShrinkToFitShapeContainerModel(ShrinkToFitShapeContainer *q)
    : q(q)
    , m_scale(1.0)
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
    Q_ASSERT(container == q); Q_UNUSED(container);
    if (type == KoShape::SizeChanged) {
        KoTextShapeData *data = dynamic_cast<KoTextShapeData *>(q->d->childShape->userData());
        Q_ASSERT(data);
        KoTextLayoutRootArea *rootArea = data->rootArea();
        Q_ASSERT(rootArea);

        QSizeF shapeSize = q->size();
        QSizeF documentSize = rootArea->boundingRect().size();
        if (m_maybeUpdate && shapeSize == m_shapeSize && documentSize == m_documentSize) {
            m_dirty = 0;
            return; // nothing to update
        }

        m_shapeSize = shapeSize;
        m_documentSize = documentSize;

        if (documentSize.width() > 0.0 && documentSize.height() > 0.0) {
            if (m_dirty || !m_maybeUpdate) {
                qreal scaleX = qMin<qreal>(1.0, shapeSize.width() / documentSize.width());
                qreal scaleY = qMin<qreal>(1.0, shapeSize.height() / documentSize.height());
                m_scale = (scaleX + scaleY) / 2.0 * 0.95;
                if (m_maybeUpdate && m_dirty) {
                    --m_dirty;
                }
            }
        } else {
            m_scale = 1.0;
            m_dirty = 1;
        }

        QSizeF newSize(shapeSize.width() / m_scale, shapeSize.height() / m_scale);
        q->d->childShape->setSize(newSize);

        QTransform m;
        m.scale(m_scale, m_scale);
        q->d->childShape->setTransformation(m);
    }
}

bool ShrinkToFitShapeContainerModel::inheritsTransform(const KoShape *child) const
{
    Q_ASSERT(child == q->d->childShape); Q_UNUSED(child);
    return true;
}

bool ShrinkToFitShapeContainerModel::isClipped(const KoShape *child) const
{
    Q_ASSERT(child == q->d->childShape); Q_UNUSED(child);
    return false;
}
