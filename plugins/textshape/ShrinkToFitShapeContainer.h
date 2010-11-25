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

#ifndef SHRINKTOFITSHAPECONTAINER_H
#define SHRINKTOFITSHAPECONTAINER_H

#include <KoShape.h>
#include <KoShapeContainer.h>
#include <SimpleShapeContainerModel.h>
#include <KoXmlNS.h>
#include <KoXmlReader.h>
#include <KoOdfLoadingContext.h>
#include <KoTextShapeData.h>
#include <QObject>
#include <QPainter>

#include <KoShapeContainer_p.h>
#include <KoTextDocumentLayout.h>
#include <KoXmlNS.h>
#include <KoOdfLoadingContext.h>
#include <KoShapeLoadingContext.h>
#include <KoResourceManager.h>

/**
 * \internal d-pointer class for the \a ShrinkToFitShapeContainer class.
 */
class ShrinkToFitShapeContainerPrivate : public KoShapeContainerPrivate
{
public:
    explicit ShrinkToFitShapeContainerPrivate(KoShapeContainer *q, KoShape *childShape) : KoShapeContainerPrivate(q), childShape(childShape) {}
    virtual ~ShrinkToFitShapeContainerPrivate() {}
    KoShape *childShape; // the original shape not owned by us
};

/**
 * Container that is used to wrap a shape and shrink a text-shape to fit the content.
 */
class ShrinkToFitShapeContainer : public KoShapeContainer
{
public:
    explicit ShrinkToFitShapeContainer(KoShape *childShape, KoResourceManager *documentResources = 0);
    virtual ~ShrinkToFitShapeContainer();

    // reimplemented
    virtual void paintComponent(QPainter &painter, const KoViewConverter &converter);
    // reimplemented
    virtual bool loadOdf(const KoXmlElement &element, KoShapeLoadingContext &context);
    // reimplemented
    virtual void saveOdf(KoShapeSavingContext &context) const;

    /**
     * Factory function to create and return a ShrinkToFitShapeContainer instance that wraps the \a shape with it.
     */
    static ShrinkToFitShapeContainer* wrapShape(KoShape *shape, KoResourceManager *documentResourceManager = 0);

    /**
     * Try to load text-on-shape from \a element and wrap \a shape with it.
     */
    static void tryWrapShape(KoShape *shape, const KoXmlElement &element, KoShapeLoadingContext &context);

    /**
     * Undo the wrapping done in the \a wrapShape method.
     */
    void unwrapShape(KoShape *shape);

    /**
     * Does update the current scale to shrink the content to proper fit if such an update is really needed. This
     * method is called by the TextShape::markLayoutDone() do be sure we always provide the possibility to update
     * if required once the complete layouting is done.
     */
    void maybeUpdateShrink();

private:
    Q_DECLARE_PRIVATE(ShrinkToFitShapeContainer)
};

/**
 * The container-model class implements \a KoShapeContainerModel for the \a ShrinkToFitShapeContainer to
 * to stuff once our container changes.
 */
class ShrinkToFitShapeContainerModel : public SimpleShapeContainerModel
{
    friend class ShrinkToFitShapeContainer;
public:
    ShrinkToFitShapeContainerModel(ShrinkToFitShapeContainer *q, ShrinkToFitShapeContainerPrivate *d);

    // reimplemented
    virtual void containerChanged(KoShapeContainer *container, KoShape::ChangeType type);
    // reimplemented
    virtual bool inheritsTransform(const KoShape *child) const;
    // reimplemented
    virtual bool isChildLocked(const KoShape *child) const;
    // reimplemented
    virtual bool isClipped(const KoShape *child) const;

private:
    ShrinkToFitShapeContainer *q;
    ShrinkToFitShapeContainerPrivate *d;
    qreal m_scaleX, m_scaleY;
    QSizeF m_shapeSize, m_documentSize;
    bool m_isDirty;
    bool m_maybeUpdate;
};

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
    if (!lay || lay->resizeMethod() != KoTextDocument::ShrinkToFitResize)
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

void ShrinkToFitShapeContainer::maybeUpdateShrink()
{
    ShrinkToFitShapeContainerModel *m = static_cast<ShrinkToFitShapeContainerModel*>(model());
    m->m_maybeUpdate = true;
    m->containerChanged(this, KoShape::SizeChanged);
    m->m_maybeUpdate = false;
}

ShrinkToFitShapeContainerModel::ShrinkToFitShapeContainerModel(ShrinkToFitShapeContainer *q, ShrinkToFitShapeContainerPrivate *d)
    : q(q)
    , d(d)
    , m_scaleX(1.0)
    , m_scaleY(1.0)
    , m_isDirty(true)
    , m_maybeUpdate(false)
{
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
            return; // nothing to update
        }

        m_shapeSize = shapeSize;
        m_documentSize = documentSize;

        if (documentSize.width() > 0.0 && documentSize.height() > 0.0) {
            if (m_isDirty || !m_maybeUpdate) {
                m_scaleX = qMin<qreal>(1.0, shapeSize.width() / documentSize.width());
                m_scaleY = qMin<qreal>(1.0, shapeSize.height() / documentSize.height());
                m_scaleX = m_scaleY = (m_scaleX+m_scaleY)/2.0 * 0.95;
            }
            m_isDirty = !m_maybeUpdate; // be sure that after a resize we always recalc again exactly once if maybeUpdate() is called
        } else { // seems layouting wasn't done yet or there is nothing we can shrink
            m_scaleX = m_scaleY = 1.0;
            m_isDirty = true; // nothing to do yet means that we still need to do something later if e.g. layouting was done
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

#endif
