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
#include <KoDocumentResourceManager.h>

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
    explicit ShrinkToFitShapeContainer(KoShape *childShape, KoDocumentResourceManager *documentResources = 0);
    virtual ~ShrinkToFitShapeContainer();

    // reimplemented
    virtual void paintComponent(QPainter &painter, const KoViewConverter &converter, KoShapePaintingContext &paintcontext);
    // reimplemented
    virtual bool loadOdf(const KoXmlElement &element, KoShapeLoadingContext &context);
    // reimplemented
    virtual void saveOdf(KoShapeSavingContext &context) const;

    /**
     * Factory function to create and return a ShrinkToFitShapeContainer instance that wraps the \a shape with it.
     */
    static ShrinkToFitShapeContainer *wrapShape(KoShape *shape, KoDocumentResourceManager *documentResourceManager = 0);

    /**
     * Try to load text-on-shape from \a element and wrap \a shape with it.
     */
    static void tryWrapShape(KoShape *shape, const KoXmlElement &element, KoShapeLoadingContext &context);

    /**
     * Undo the wrapping done in the \a wrapShape method.
     */
    void unwrapShape(KoShape *shape);

private:
    Q_DECLARE_PRIVATE(ShrinkToFitShapeContainer)
};

/**
 * The container-model class implements \a KoShapeContainerModel for the \a ShrinkToFitShapeContainer to
 * to stuff once our container changes.
 */
class ShrinkToFitShapeContainerModel : public QObject, public SimpleShapeContainerModel
{
    Q_OBJECT
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

private Q_SLOTS:
    void finishedLayout();

private:
    ShrinkToFitShapeContainer *q;
    ShrinkToFitShapeContainerPrivate *d;
    qreal m_scale;
    QSizeF m_shapeSize, m_documentSize;
    int m_dirty;
    bool m_maybeUpdate;
};

#endif
