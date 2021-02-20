/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2010 Thomas Zander <zander@kde.org>
 * SPDX-FileCopyrightText: 2010 KO GmbH <boud@kogbmh.com>
 * SPDX-FileCopyrightText: 2010 Thorsten Zachmann <zachmann@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KOTOSCONTAINER_H
#define KOTOSCONTAINER_H

#include "KoShapeContainer.h"

#include "kritaflake_export.h"

class KoDocumentResourceManager;


/**
 * Container that is used to wrap a shape with a text on top.
 * Path shapes inherit from this class to make it possible to have text associated
 * with them.
 */
class KRITAFLAKE_EXPORT KoTosContainer : public KoShapeContainer
{
public:
    KoTosContainer();
    ~KoTosContainer() override;

    // reimplemented
    void paintComponent(QPainter &painter, KoShapePaintingContext &paintcontext) const override;

    // reimplemented
    virtual bool loadText(const KoXmlElement &element, KoShapeLoadingContext &context);


    /// different kinds of resizing behavior to determine how to treat text overflow
    enum ResizeBehavior {
        TextFollowsSize,  ///< Text area is same size as content, extra text will be clipped
        FollowTextSize,   ///< Content shape will get resized if text grows/shrinks
        IndependentSizes,  ///< The text can get bigger than the content
        TextFollowsPreferredTextRect ///< The size/position of the text area will follow the preferredTextRect property
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
     *
     * @param documentResources
     * @return The created text shape or 0 in case it failed
     */
    KoShape *createTextShape(KoDocumentResourceManager *documentResources = 0);

    void setRunThrough(short int runThrough) override;

protected:
    KoTosContainer(const KoTosContainer &rhs);

        /**
     * Set the current preferred text rectangle. This rect contains the coordinates of
     * the embedded text shape relative to the content shape. This value is ignored if
     * resizeBehavior is not TextFollowsPreferredTextRect.
     * @param rect the new preferred text rectangle
     */
    void setPreferredTextRect(const QRectF &rect);

    /**
     * Returns the current preferred text rectangle.
     */
    QRectF preferredTextRect() const;

    /**
     * Returns the text shape
     *
     * @returns textshape if set or 0 in case it is not yet set
     */
    KoShape *textShape() const;

    void shapeChanged(ChangeType type, KoShape *shape = 0) override;

private:
    class Private;
    QSharedDataPointer<Private> d;
};

#endif
