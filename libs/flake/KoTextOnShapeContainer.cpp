/* This file is part of the KDE project
 * Copyright (C) 2010 Thomas Zander <zander@kde.org>
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

#include "KoTextOnShapeContainer.h"
#include "KoShapeContainer_p.h"
#include "SimpleShapeContainerModel.h"
#include "KoShapeRegistry.h"
#include "KoShapeFactoryBase.h"
#include "KoTextShapeDataBase.h"

#include <KDebug>

class KoTextOnShapeContainerPrivate : public KoShapeContainerPrivate
{
public:
    KoTextOnShapeContainerPrivate(KoShapeContainer *q);
    virtual ~KoTextOnShapeContainerPrivate();

    KoShape *content; // the original shape
    KoShape *textShape;
};

class KoTextOnShapeContainerModel : public SimpleShapeContainerModel
{
public:
    KoTextOnShapeContainerModel(KoTextOnShapeContainer *qq, KoTextOnShapeContainerPrivate *containerData);
    virtual void containerChanged(KoShapeContainer *container);
    virtual void proposeMove(KoShape *child, QPointF &move);
    virtual void childChanged(KoShape *child, KoShape::ChangeType type);

    KoTextOnShapeContainer *q;
    KoTextOnShapeContainerPrivate *containerData;
    bool lock;
};

// KoTextOnShapeContainerPrivate
KoTextOnShapeContainerPrivate::KoTextOnShapeContainerPrivate(KoShapeContainer *q)
    : KoShapeContainerPrivate(q),
    content(0),
    textShape(0)
{
}

KoTextOnShapeContainerPrivate::~KoTextOnShapeContainerPrivate()
{
    // the 'content' object is not owned by us
    delete textShape;
}

/// KoTextOnShapeContainerModel
KoTextOnShapeContainerModel::KoTextOnShapeContainerModel(KoTextOnShapeContainer *qq, KoTextOnShapeContainerPrivate *data)
    : q(qq),
    containerData(data),
    lock(false)
{
}

void KoTextOnShapeContainerModel::containerChanged(KoShapeContainer *container)
{
    // TODO
}

void KoTextOnShapeContainerModel::proposeMove(KoShape *child, QPointF &move)
{
    if (child == containerData->textShape) { // not user movable
        move.setX(0);
        move.setY(0);
    }
}

void KoTextOnShapeContainerModel::childChanged(KoShape *child, KoShape::ChangeType type)
{
    if (lock)
        return;
    lock = true;
    // the 'content' object matrix is leading
    /*
        To make this working correctly while interacting with the shape we need to have
        user input support in the model as we can't change the parent or child properties here
        otherwise.
     */
    if (child == containerData->content) {
        KoShape *text = containerData->textShape;
        switch (type) {
        case KoShape::PositionChanged:
//           q->setPosition(child->position());
//           child->setPosition(QPointF());
            if (text)
                text->setPosition(child->position());
            break;
        case KoShape::SizeChanged:
            q->setSize(child->size());
            if (text)
                text->setSize(child->size());
            break;
        case KoShape::RotationChanged:
        case KoShape::ScaleChanged:
        case KoShape::ShearChanged:
        case KoShape::GenericMatrixChange: {
            //q->setTransformation(child->transformation());
            //child->setTransformation(QMatrix());
            if (text)
                text->setTransformation(child->transformation());
            break;
        }
        case KoShape::ParentChanged: // TODO panic :)
            break;

        case KoShape::Deleted: // TODO panic :)
            break;

        case KoShape::ShadowChanged:
        case KoShape::BorderChanged:
        case KoShape::ParameterChanged:
            break;

        default: // the others are not interesting for us
            break;
        }
    }
    lock = false;
}

/// KoTextOnShapeContainer
KoTextOnShapeContainer::KoTextOnShapeContainer(KoShape *childShape)
    : KoShapeContainer(*(new KoTextOnShapeContainerPrivate(this)))
{
    Q_D(KoTextOnShapeContainer);
    Q_ASSERT(childShape);
    d->content = childShape;
    d->model = new KoTextOnShapeContainerModel(this, d);

    //setPosition(childShape->position());
    setSize(childShape->size());
    setZIndex(childShape->zIndex());
    setTransformation(childShape->transformation());
    //childShape->setPosition(QPointF()); // since its relative to my position, this won't move it
    setTransformation(QMatrix());

    QSet<KoShape*> delegates;
    delegates << childShape;
    KoShapeFactoryBase *factory = KoShapeRegistry::instance()->get("TextShapeID");
    if (factory) { // not installed, thats too bad, but allowed
        d->textShape = factory->createDefaultShape(0);
        Q_ASSERT(d->textShape); // would be a bug in the text shape;
        d->textShape->setSize(size());
        d->textShape->setPosition(childShape->position());
        d->textShape->setTransformation(childShape->transformation());
        KoTextShapeDataBase *shapeData = qobject_cast<KoTextShapeDataBase*>(d->textShape->userData());
        Q_ASSERT(shapeData); // would be a bug in kotext
        shapeData->setVerticalAlignment(Qt::AlignVCenter);
        addShape(d->textShape);
        d->textShape->setZIndex(childShape->zIndex() + 1);
        delegates << d->textShape;
    } else {
        kWarning(30006) << "Text shape factory not found";
    }
    addShape(childShape);
    setToolDelegates(delegates);
}

void KoTextOnShapeContainer::paintComponent(QPainter &, const KoViewConverter &)
{
}

bool KoTextOnShapeContainer::loadOdf(const KoXmlElement &element, KoShapeLoadingContext &context)
{
    // TODO
    return false;
}

void KoTextOnShapeContainer::saveOdf(KoShapeSavingContext &context) const
{
    // TODO
}

void KoTextOnShapeContainer::setPlainText(const QString &text)
{
    Q_D(KoTextOnShapeContainer);
    if (d->textShape == 0) {
        kWarning(30006) << "No text shape present in KoTextOnShapeContainer";
        return;
    }
    KoTextShapeDataBase *shapeData = qobject_cast<KoTextShapeDataBase*>(d->textShape->userData());
    Q_ASSERT(shapeData->document());
    shapeData->document()->setPlainText(text);
}
