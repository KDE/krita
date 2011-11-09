/* This file is part of the KDE project
 * Copyright (C) 2010 Thomas Zander <zander@kde.org>
 * Copyright (C) 2010 KO GmbH <boud@kogmbh.com>
 * Copyright (C) 2010 Thorsten Zachmann <zachmann@kde.org>
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

#include "KoTosContainer.h"

#include "KoTosContainer_p.h"
#include "KoShapeRegistry.h"
#include "KoShapeFactoryBase.h"
#include "KoShapeLoadingContext.h"
#include "KoTextShapeDataBase.h"
#include "KoTosContainerModel.h"

#include <QTextCursor>

KoTosContainerPrivate::KoTosContainerPrivate(KoShapeContainer *q)
    : KoShapeContainerPrivate(q)
    , resizeBehavior(KoTosContainer::IndependentSizes)
{
}

KoTosContainerPrivate::~KoTosContainerPrivate()
{
}


KoTosContainer::KoTosContainer()
    : KoShapeContainer(*(new KoTosContainerPrivate(this)))
{
}

KoTosContainer::~KoTosContainer()
{
    delete textShape();
}

KoTosContainer::KoTosContainer(KoTosContainerPrivate &dd)
    : KoShapeContainer(dd)
{
}

void KoTosContainer::paintComponent(QPainter &, const KoViewConverter &, KoShapePaintingContext &)
{
}

bool KoTosContainer::loadText(const KoXmlElement &element, KoShapeLoadingContext &context)
{
    KoXmlElement child;
    forEachElement(child, element) {
        // only recreate the text shape if there's something to be loaded
        if (child.localName() == "p" || child.localName() == "list") {

            KoShape *textShape = createTextShape(context.documentResourceManager());
            if (!textShape) {
                return false;
            }

            // In the case of text on shape, we cannot ask the text shape to load
            // the odf, since it expects a complete document with style info and
            // everything, so we have to use the KoTextShapeData object instead.
            KoTextShapeDataBase *shapeData = qobject_cast<KoTextShapeDataBase*>(textShape->userData());
            Q_ASSERT(shapeData);
            bool loadOdf = shapeData->loadOdf(element, context);
            return loadOdf;
        }
    }
    return true;
}

void KoTosContainer::saveText(KoShapeSavingContext &context) const
{
    KoShape *textShape = this->textShape();
    if (!textShape) {
        return;
    }
    // In the case of text on shape, we cannot ask the text shape to save
    // the odf, since it would save all the frame information as well, which
    // is wrong.
    // Only save the text shape if it has content.
    KoTextShapeDataBase *shapeData = qobject_cast<KoTextShapeDataBase*>(textShape->userData());
    if (shapeData && !shapeData->document()->isEmpty()) {
        shapeData->saveOdf(context);
    }
}

void KoTosContainer::setPlainText(const QString &text)
{
    KoShape *textShape = this->textShape();
    if (textShape == 0) {
        kWarning(30006) << "No text shape present in KoTosContainer";
        return;
    }
    KoTextShapeDataBase *shapeData = qobject_cast<KoTextShapeDataBase*>(textShape->userData());
    Q_ASSERT(shapeData->document());
    shapeData->document()->setPlainText(text);
}

void KoTosContainer::setResizeBehavior(ResizeBehavior resizeBehavior)
{
    Q_D(KoTosContainer);
    if (d->resizeBehavior == resizeBehavior) {
        return;
    }
    d->resizeBehavior = resizeBehavior;
    if (d->model) {
        d->model->containerChanged(this, KoShape::SizeChanged);
    }
}

KoTosContainer::ResizeBehavior KoTosContainer::resizeBehavior() const
{
    Q_D(const KoTosContainer);
    return d->resizeBehavior;
}

void KoTosContainer::setTextAlignment(Qt::Alignment alignment)
{
    KoShape *textShape = this->textShape();
    if (textShape == 0) {
        kWarning(30006) << "No text shape present in KoTosContainer";
        return;
    }

    // vertical
    KoTextShapeDataBase *shapeData = qobject_cast<KoTextShapeDataBase*>(textShape->userData());
    shapeData->setVerticalAlignment(alignment);

    // horizontal
    Q_ASSERT(shapeData->document());
    QTextBlockFormat bf;
    bf.setAlignment(alignment & Qt::AlignHorizontal_Mask);

    QTextCursor cursor(shapeData->document());
    cursor.setPosition(QTextCursor::End, QTextCursor::KeepAnchor);
    cursor.mergeBlockFormat(bf);
}

Qt::Alignment KoTosContainer::textAlignment() const
{
    KoShape *textShape = this->textShape();
    if (textShape == 0) {
        kWarning(30006) << "No text shape present in KoTosContainer";
        return Qt::AlignTop;
    }

    // vertical
    KoTextShapeDataBase *shapeData = qobject_cast<KoTextShapeDataBase*>(textShape->userData());
    // the model makes sure it contains a shape that has a KoTextShapeDataBase set so no need to check that
    Qt::Alignment answer = shapeData->verticalAlignment() & Qt::AlignVertical_Mask;

    // horizontal
    Q_ASSERT(shapeData->document());
    QTextCursor cursor(shapeData->document());
    answer = answer | (cursor.blockFormat().alignment() & Qt::AlignHorizontal_Mask);

    return answer;
}

void KoTosContainer::setPreferredTextRect(const QRectF &rect)
{
    Q_D(KoTosContainer);
    d->preferredTextRect = rect;
    KoShape *textShape = this->textShape();
    //kDebug(30006) << rect << textShape << d->resizeBehavior;
    if (d->resizeBehavior == TextFollowsPreferredTextRect && textShape) {
        //kDebug(30006) << rect;
        textShape->setPosition(rect.topLeft());
        textShape->setSize(rect.size());
    }
}

QRectF KoTosContainer::preferredTextRect() const
{
    Q_D(const KoTosContainer);
    return d->preferredTextRect;
}

KoShape *KoTosContainer::createTextShape(KoDocumentResourceManager *documentResources)
{
    if (!documentResources) {
        kWarning(30006) << "KoDocumentResourceManager not found";
        return 0;
    }

    Q_D(KoTosContainer);

    delete textShape();
    delete d->model;

    d->model = new KoTosContainerModel();

    QSet<KoShape*> delegates;
    delegates << this;
    KoShape *textShape = 0;
    KoShapeFactoryBase *factory = KoShapeRegistry::instance()->get("TextShapeID");
    if (factory) { // not installed, thats too bad, but allowed
        textShape = factory->createDefaultShape(documentResources);
        Q_ASSERT(textShape); // would be a bug in the text shape;
        if (d->resizeBehavior == TextFollowsPreferredTextRect) {
            textShape->setSize(d->preferredTextRect.size());
        } else {
            textShape->setSize(size());
        }
        textShape->setTransformation(transformation());
        if (d->resizeBehavior == TextFollowsPreferredTextRect) {
            textShape->setPosition(d->preferredTextRect.topLeft());
        } else {
            textShape->setPosition(QPointF(0, 0));
        }
        textShape->setSelectable(false);
        textShape->setRunThrough(runThrough());
        KoTextShapeDataBase *shapeData = qobject_cast<KoTextShapeDataBase*>(textShape->userData());
        Q_ASSERT(shapeData); // would be a bug in kotext
        // TODO check if that is correct depending on the resize mode
        shapeData->setVerticalAlignment(Qt::AlignVCenter);
        addShape(textShape);
        // textShape->setZIndex(zIndex() + 1); // not needed as there as the text shape is the only sub shape
        delegates << textShape;
    } else {
        kWarning(30006) << "Text shape factory not found";
    }
    setToolDelegates(delegates);
    return textShape;
}

KoShape *KoTosContainer::textShape() const
{
    const QList<KoShape*> subShapes = shapes();
    return subShapes.isEmpty() ? 0 : subShapes.at(0);
}

void KoTosContainer::shapeChanged(ChangeType type, KoShape *shape)
{
    Q_UNUSED(shape);
    Q_D(KoTosContainer);
    if (d->model == 0) {
        return;
    }

    if (type == SizeChanged || type == ContentChanged) {
        d->model->containerChanged(this, type);
    }
    // TODO is this needed?
#if 0
    foreach(KoShape *shape, d->model->shapes())
        shape->notifyChanged();
#endif
}

void KoTosContainer::setRunThrough(short int runThrough)
{
    KoShape::setRunThrough(runThrough);
    KoShape *textShape = this->textShape();
    if (textShape) {
        textShape->setRunThrough(runThrough);
    }
}
