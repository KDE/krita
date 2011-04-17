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
    if (!data || data->resizeMethod() != KoTextShapeData::ShrinkToFitResize)
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
    , m_scale(1.0)
    , m_dueToLayout(false)
    , m_changeCount(0)
{
}

void ShrinkToFitShapeContainerModel::finishedLayout()
{
    qDebug()<<"container got a finishedlayout";
    m_dueToLayout = true;
    containerChanged(q, KoShape::SizeChanged);
    m_dueToLayout = false;
}

void ShrinkToFitShapeContainerModel::containerChanged(KoShapeContainer *container, KoShape::ChangeType type)
{
    Q_ASSERT(container == q);
    if (type == KoShape::SizeChanged) {
        KoTextShapeData* data = dynamic_cast<KoTextShapeData*>(d->childShape->userData());
        Q_ASSERT(data);
        KoTextLayoutRootArea *rootArea = data->rootArea();
        Q_ASSERT(rootArea);

        QSizeF shapeSize = q->size();
        QSizeF documentSize = rootArea->boundingRect().size();

        if (shapeSize == m_shapeSize && documentSize == m_documentSize) {
            m_changeCount = 0;
            return; // nothing to update
        }

        m_shapeSize = shapeSize;
        m_documentSize = documentSize;

        if (m_dueToLayout && m_changeCount>0 && documentSize.height()*m_scale <= shapeSize.height()) {
            m_changeCount = 0;
            return;
        }

        m_scale = 1.0;
        if ( documentSize.height() > 0.0) {
            m_scale = qMin<qreal>(1.0, shapeSize.height() / documentSize.height());
        }

        m_changeCount++;

        d->childShape->setSize(QSizeF(shapeSize.width() / m_scale, shapeSize.height() / m_scale));

        QTransform m;
        m.scale(m_scale, m_scale);
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
