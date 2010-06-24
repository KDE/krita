/* This file is part of the KDE project
 * Copyright (C) 2010 Thomas Zander <zander@kde.org>
 * Copyright (C) 2010 KO GmbH <boud@kogbmh.com>
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
 * Path shapes inherit from this class to make it possible to have text associated
 * with them.
 */
class FLAKE_EXPORT KoTextOnShapeContainer : public KoShapeContainer
{
public:

    // reimplemented
    virtual void paintComponent(QPainter &painter, const KoViewConverter &converter);
    // reimplemented
    virtual bool loadText(const KoXmlElement &element, KoShapeLoadingContext &context);
    // reimplemented
    virtual void saveText(KoShapeSavingContext &context) const;

    /// different kinds of resizing behavior to determine how to treat text overflow
    enum ResizeBehavior {
        TextFollowsSize,  ///< Text area is same size as content, extra text will be clipped
        FollowTextSize,   ///< Content shape will get resized if text grows/shrinks
        IndependentSizes  ///< The text can get bigger than the content
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


    /**
     * Add text the current shape with the specified document resource manager.
     */
    void createTextShape(KoResourceManager *documentResources = 0);


protected:

    /// constructor
    KoTextOnShapeContainer(KoTextOnShapeContainerPrivate &);

private:

    Q_DECLARE_PRIVATE(KoTextOnShapeContainer)
};

#endif
