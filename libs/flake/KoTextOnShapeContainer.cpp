/* This file is part of the KDE project
 * Copyright (C) 2010 Thomas Zander <zander@kde.org>
 * Copyright (C) 2010 KO GmbH <boud@kogmbh.com>
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
#include "KoTextOnShapeContainer_p.h"
#include "KoShapeContainer_p.h"
#include "SimpleShapeContainerModel.h"
#include "KoShapeRegistry.h"
#include "KoShapeFactoryBase.h"
#include "KoTextShapeDataBase.h"
#include "KoShapeLoadingContext.h"
#include "KoShapeSavingContext.h"

#include <KDebug>
#include <QTextCursor>

// KoTextOnShapeContainerPrivate
KoTextOnShapeContainerPrivate::KoTextOnShapeContainerPrivate(KoShapeContainer *q)
    : KoShapeContainerPrivate(q),
    textShape(0),
    resizeBehavior(KoTextOnShapeContainer::IndependentSizes)
{
}

KoTextOnShapeContainerPrivate::~KoTextOnShapeContainerPrivate()
{
    delete textShape;
}

/// KoTextOnShapeContainerModel
KoTextOnShapeContainerModel::KoTextOnShapeContainerModel(KoTextOnShapeContainer *qq,
                                                         KoTextOnShapeContainerPrivate *data)
    : q(qq)
    , containerData(data)
    , lock(false)
{
}

void KoTextOnShapeContainerModel::containerChanged(KoShapeContainer *container, KoShape::ChangeType type)
{
    if (lock || type != KoShape::SizeChanged) {
        return;
    }
    lock = true;
    Q_ASSERT(container == q);
    KoShape *text = containerData->textShape;
    if (text) {
        text->setSize(q->size());
    }
    lock = false;
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
    if (lock) {
        return;
    }
    lock = true;
    // the container is leading in size, so the only reason we get here is when
    // one of the child shapes decided to resize itself. This would probably be
    // the text shape deciding it needs more space.
    KoShape *text = containerData->textShape;
    if (child == text) {
        switch (type) {
        case KoShape::SizeChanged:
            q->setSize(text->size()); // should have a policy to decide what to do
            break;
        default: // the others are not interesting for us
            break;
        }
    }
    lock = false;
}

bool KoTextOnShapeContainerModel::inheritsTransform(const KoShape *) const {
    return true;
}


/// KoTextOnShapeContainer

KoTextOnShapeContainer::KoTextOnShapeContainer(KoTextOnShapeContainerPrivate &dd)
    : KoShapeContainer(dd)
{
}

void KoTextOnShapeContainer::paintComponent(QPainter &, const KoViewConverter &)
{
}

bool KoTextOnShapeContainer::loadText(const KoXmlElement &element, KoShapeLoadingContext &context)
{
    Q_D(KoTextOnShapeContainer);

    KoXmlElement child;
    forEachElement(child, element) {
        // only recreate the text shape if there's something to be loaded
        if (child.localName() == "p") {

            createTextShape(context.documentResourceManager());

            // In the case of text on shape, we cannot ask the text shape to load
            // the odf, since it expects a complete document with style info and
            // everything, so we have to use the KoTextShapeData object instead.
            KoTextShapeDataBase *shapeData
                    = qobject_cast<KoTextShapeDataBase*>(d->textShape->userData());
            if (shapeData) {
                bool loadOdf  = shapeData->loadOdf(element, context);

                d->textShape->setSize(size());

                d->textShape->setTransformation(transformation());
                d->textShape->setPosition(QPointF(0, 0));
                setTextAlignment(Qt::AlignCenter);
                return loadOdf;
            }
        }
    }
    return false;
}

void KoTextOnShapeContainer::saveText(KoShapeSavingContext &context) const
{
    Q_D(const KoTextOnShapeContainer);
    if (!d->textShape) {
        return;
    }
    // In the case of text on shape, we cannot ask the text shape to save
    // the odf, since it would save all the frame information as well, which
    // is wrong.
    KoTextShapeDataBase *shapeData
            = qobject_cast<KoTextShapeDataBase*>(d->textShape->userData());
    if (shapeData && !shapeData->document()->isEmpty()) {
        shapeData->saveOdf(context);
    }
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

void KoTextOnShapeContainer::setResizeBehavior(ResizeBehavior resizeBehavior)
{
    Q_D(KoTextOnShapeContainer);
    if (d->resizeBehavior == resizeBehavior) {
        return;
    }
    d->resizeBehavior = resizeBehavior;
    d->model->containerChanged(this, KoShape::SizeChanged);
}

KoTextOnShapeContainer::ResizeBehavior KoTextOnShapeContainer::resizeBehavior() const
{
    Q_D(const KoTextOnShapeContainer);
    return d->resizeBehavior;
}

void KoTextOnShapeContainer::setTextAlignment(Qt::Alignment alignment)
{
    Q_D(KoTextOnShapeContainer);
    if (d->textShape == 0) {
        kWarning(30006) << "No text shape present in KoTextOnShapeContainer";
        return;
    }

    // vertical
    KoTextShapeDataBase *shapeData = qobject_cast<KoTextShapeDataBase*>(d->textShape->userData());
    shapeData->setVerticalAlignment(alignment);

    // horizontal
    Q_ASSERT(shapeData->document());
    QTextBlockFormat bf;
    bf.setAlignment(alignment & Qt::AlignHorizontal_Mask);

    QTextCursor cursor(shapeData->document());
    cursor.setPosition(QTextCursor::End, QTextCursor::KeepAnchor);
    cursor.mergeBlockFormat(bf);
}

Qt::Alignment KoTextOnShapeContainer::textAlignment() const
{
    Q_D(const KoTextOnShapeContainer);
    if (d->textShape == 0) {
        kWarning(30006) << "No text shape present in KoTextOnShapeContainer";
        return Qt::AlignTop;
    }

    // vertical
    KoTextShapeDataBase *shapeData = qobject_cast<KoTextShapeDataBase*>(d->textShape->userData());
    Qt::Alignment answer = shapeData->verticalAlignment() & Qt::AlignVertical_Mask;

    // horizontal
    Q_ASSERT(shapeData->document());
    QTextCursor cursor(shapeData->document());
    answer = answer | (cursor.blockFormat().alignment() & Qt::AlignHorizontal_Mask);

    return answer;
}

void KoTextOnShapeContainer::createTextShape(KoResourceManager *documentResources)
{

    if (!documentResources) {
        return;
    }

    Q_D(KoTextOnShapeContainer);

    delete d->textShape;
    delete d->model;

    d->model = new KoTextOnShapeContainerModel(this, d);

    QSet<KoShape*> delegates;
    delegates << this;
    KoShapeFactoryBase *factory = KoShapeRegistry::instance()->get("TextShapeID");
    if (factory) { // not installed, thats too bad, but allowed
        d->textShape = factory->createDefaultShape(documentResources);
        Q_ASSERT(d->textShape); // would be a bug in the text shape;
        d->textShape->setSize(size());
        d->textShape->setTransformation(transformation());
        KoTextShapeDataBase *shapeData = qobject_cast<KoTextShapeDataBase*>(d->textShape->userData());
        Q_ASSERT(shapeData); // would be a bug in kotext
        shapeData->setVerticalAlignment(Qt::AlignVCenter);
        addShape(d->textShape);
        d->textShape->setZIndex(zIndex() + 1);
        d->textShape->setSelectable(false);
        delegates << d->textShape;
    } else {
        kWarning(30006) << "Text shape factory not found";
    }
    setToolDelegates(delegates);
}
