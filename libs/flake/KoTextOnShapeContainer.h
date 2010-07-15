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

#ifndef KOTEXTONSHAPECONTAINER_H
#define KOTEXTONSHAPECONTAINER_H

#include "KoShapeContainer.h"

#include "flake_export.h"

class KoTextOnShapeContainerPrivate;
class KoResourceManager;

/**
 * Container that is used to wrap a shape with a text on top.
 * Adding this container as a parent to any shape will allow you to add text
 * on top of that shape in the form of the decorator (design) pattern.
 */
class FLAKE_EXPORT KoTextOnShapeContainer : public KoShapeContainer
{
public:
    /**
     * Constructor to make a decorator container that adds a text on top of another shape.
     * @param childShape this is the original shape that should get the text placed on top of.
     * @param documentResources a resource manager holding properties for the text shape to
     *      integrate better into the document. Not passing this will mean that for example
     *      the text styles will not be shared with other shapes.
     */
    explicit KoTextOnShapeContainer(KoShape *childShape, KoResourceManager *documentResources = 0);

    // reimplemented
    virtual void paintComponent(QPainter &painter, const KoViewConverter &converter);
    // reimplemented
    virtual bool loadOdf(const KoXmlElement &element, KoShapeLoadingContext &context);
    // reimplemented
    virtual void saveOdf(KoShapeSavingContext &context) const;

    /// different kinds of resizing behavior to determine how to treat text overflow
    enum ResizeBehavior {
        TextFollowsContentSize, ///< Text area is same size as content, extra text will be clipped
        ContentFollowsTextSize, ///< Content shape will get resized if text grows/shrinks
        IndependendSizes        ///< The text can get bigger than the content
    };

    /**
     * Set the behavior that is used to resize the text or content.
     * In order to determine what to do when there is too much text to fit or suddenly less
     * text the user can define the wanted behavior using the ResizeBehavior
     * @param resizeBehavior the new ResizeBehavior
     */
    void setResizeBehavior(ResizeBehavior resizeBehavior);

    /**
     * Returns the current ResizeBehavior.
     */
    ResizeBehavior resizeBehavior() const;

    /** Sets the alignment of the text. */
    void setTextAlignment(Qt::Alignment alignment);
    /** Returns the alignment of all text */
    Qt::Alignment textAlignment() const;

    /**
     * Set some plain text to be displayed on the shape.
     * @param text the full text.
     */
    void setPlainText(const QString &text);

private:
    Q_DECLARE_PRIVATE(KoTextOnShapeContainer)
};

#endif
