/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2010 Thomas Zander <zander@kde.org>
 * SPDX-FileCopyrightText: 2010 KO GmbH <boud@valdyas.org>
 * SPDX-FileCopyrightText: 2010 Thorsten Zachmann <zachmann@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "KoTosContainer.h"

#include "KoTosContainer_p.h"
#include "KoShapeRegistry.h"
#include "KoShapeFactoryBase.h"
#include "KoShapeLoadingContext.h"
#include "KoTextShapeDataBase.h"
#include "KoTosContainerModel.h"
#include "KoXmlNS.h"

#include <FlakeDebug.h>

#include <QTextCursor>
#include <QTextDocument>

KoTosContainer::Private::Private()
    : QSharedData()
    , resizeBehavior(KoTosContainer::IndependentSizes)
{
}

KoTosContainer::Private::Private(const Private &rhs)
    : QSharedData()
    , resizeBehavior(rhs.resizeBehavior)
    , preferredTextRect(rhs.preferredTextRect)
    , alignment(rhs.alignment)
{
}

KoTosContainer::Private::~Private()
{
}


KoTosContainer::KoTosContainer()
    : KoShapeContainer()
    , d(new Private)
{
}

KoTosContainer::KoTosContainer(const KoTosContainer &rhs)
    : KoShapeContainer(rhs)
    , d(rhs.d)
{
}

KoTosContainer::~KoTosContainer()
{
    delete textShape();
}

void KoTosContainer::paintComponent(QPainter &, KoShapePaintingContext &) const
{
}

bool KoTosContainer::loadText(const KoXmlElement &element, KoShapeLoadingContext &context)
{
    Q_UNUSED(element);
    Q_UNUSED(context);
    return false;
}


void KoTosContainer::setPlainText(const QString &text)
{
    KoShape *textShape = this->textShape();
    if (textShape == 0) {
        warnFlake << "No text shape present in KoTosContainer";
        return;
    }
    KoTextShapeDataBase *shapeData = qobject_cast<KoTextShapeDataBase*>(textShape->userData());
    Q_ASSERT(shapeData->document());
    shapeData->document()->setPlainText(text);
}

void KoTosContainer::setResizeBehavior(ResizeBehavior resizeBehavior)
{
    if (d->resizeBehavior == resizeBehavior) {
        return;
    }
    d->resizeBehavior = resizeBehavior;
    if (model()) {
        model()->containerChanged(this, KoShape::SizeChanged);
    }
}

KoTosContainer::ResizeBehavior KoTosContainer::resizeBehavior() const
{
    return d->resizeBehavior;
}

void KoTosContainer::setTextAlignment(Qt::Alignment alignment)
{
    KoShape *textShape = this->textShape();
    if (textShape == 0) {
        warnFlake << "No text shape present in KoTosContainer";
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
    cursor.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
    cursor.mergeBlockFormat(bf);

    d->alignment = alignment;
}

Qt::Alignment KoTosContainer::textAlignment() const
{
    KoShape *textShape = this->textShape();
    if (textShape == 0) {
        warnFlake << "No text shape present in KoTosContainer";
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
    d->preferredTextRect = rect;
    KoShape *textShape = this->textShape();
    //debugFlake << rect << textShape << d->resizeBehavior;
    if (d->resizeBehavior == TextFollowsPreferredTextRect && textShape) {
        //debugFlake << rect;
        textShape->setPosition(rect.topLeft());
        textShape->setSize(rect.size());
    }
}

QRectF KoTosContainer::preferredTextRect() const
{
    return d->preferredTextRect;
}

KoShape *KoTosContainer::createTextShape(KoDocumentResourceManager *documentResources)
{
    if (!documentResources) {
        warnFlake << "KoDocumentResourceManager not found";
        return 0;
    }

    delete textShape();
    delete model();

    setModel(new KoTosContainerModel());

    QSet<KoShape*> delegates;
    delegates << this;
    KoShape *textShape = 0;
    KoShapeFactoryBase *factory = KoShapeRegistry::instance()->get("TextShapeID");
    if (factory) { // not installed, that's too bad, but allowed
        textShape = factory->createDefaultShape(documentResources);
        Q_ASSERT(textShape); // would be a bug in the text shape;
        if (d->resizeBehavior == TextFollowsPreferredTextRect) {
            textShape->setSize(d->preferredTextRect.size());
        } else {
            textShape->setSize(size());
        }
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
        warnFlake << "Text shape factory not found";
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
    if (model() == 0) {
        return;
    }

    if (type == SizeChanged || type == ContentChanged) {
        model()->containerChanged(this, type);
    }
    // TODO is this needed?
#if 0
    Q_FOREACH (KoShape *shape, model()->shapes())
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
