/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2007, 2009 Thomas Zander <zander@kde.org>
 * SPDX-FileCopyrightText: 2011 Matus Hanzes <matus.hanzes@ixonos.com>
 * SPDX-FileCopyrightText: 2013 C. Boemann <cbo@boemann.dk>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */
#ifndef KOSHAPEANCHOR_H
#define KOSHAPEANCHOR_H

#include "kritaflake_export.h"


class KoShape;
#include <KoXmlReaderForward.h>
class KoShapeLoadingContext;
class KoShapeSavingContext;
class KoShapeAnchorPrivate;

class QTextDocument;
class QPointF;
class QString;

/**
 * This class is the object that explains how a shape is anchored to something.
 *
 * The anchored shape will be positioned (in supporting applications) based on the properties
 * defined in this class.
 *
 * This class can be used in three different ways:
 *  -page anchor
 *  -as-char
 *  -char, paragraph anchor
 *
 * If it's a page anchor it just provide the info about how the shape relates to a page with a specific
 * page number.
 *
 * For the other types of anchoring it has to have a TextLocation in a QTextDocument. This TextLocation
 * can either be an inline character (type as-char) or a position (type char or paragraph) The
 * KoShapeAnchor and TextLocation connects the anchored-shape to the text flow so the anchored shape
 * can be repositioned on the canvas if new text is inserted or removed before the anchor character.
 *
 * For as-char, char and paragraph use cases:
 * @see KoAnchorInlineObject
 * @see KoAnchorTextRange
 * which are both implemented as subclasses of TextLocation
 *
 * The position of the shape relative to the anchor is called the offset.
 * @see PlacementStrategy for more information about the layout of anchors/shapes.
 */
class KRITAFLAKE_EXPORT KoShapeAnchor
{
public:
    /**
    * This class is an interface that positions the shape linked to text anchor
    */
    class PlacementStrategy {
    public:
        PlacementStrategy(){};
        virtual ~PlacementStrategy(){};

        /**
         * Reparent the anchored shape to not have a parent shape container (and model)
         *
         */
        virtual void detachFromModel() = 0;

        /**
         * Reparent the anchored shape under an appropriate shape container (and model)
         *
         * If needed, it changes the parent KoShapeContainerModel and KoShapeContainer of the anchored
         * shape.
         */
        virtual void updateContainerModel() = 0;
    };

    class TextLocation {
    public:
        TextLocation(){};
        virtual ~TextLocation(){};
        virtual const QTextDocument *document() const = 0;
        virtual int position() const = 0;
    };

    enum HorizontalPos {
        HCenter,
        HFromInside,
        HFromLeft,
        HInside,
        HLeft,
        HOutside,
        HRight
    };

    enum HorizontalRel { //NOTE: update KWAnchoringProperties if you change this
        HChar,
        HPage,
        HPageContent,
        HPageStartMargin,
        HPageEndMargin,
        HFrame,
        HFrameContent,
        HFrameEndMargin,
        HFrameStartMargin,
        HParagraph,
        HParagraphContent,
        HParagraphEndMargin,
        HParagraphStartMargin
    };

    enum VerticalPos {
        VBelow,
        VBottom,
        VFromTop,
        VMiddle,
        VTop
    };

    enum VerticalRel { //NOTE: update KWAnchoringProperties if you change this
        VBaseline,
        VChar,
        VFrame,
        VFrameContent,
        VLine,
        VPage,
        VPageContent,
        VParagraph,
        VParagraphContent,
        VText
    };

    enum AnchorType {
        AnchorAsCharacter,
        AnchorToCharacter,
        AnchorParagraph,
        AnchorPage
    };

    /**
     * Constructor for an in-place anchor.
     * @param shape the anchored shape that this anchor links to.
     */
    explicit KoShapeAnchor(KoShape *shape);
    virtual ~KoShapeAnchor();

    /**
     * Return the shape that is linked to from the text anchor.
     */
    KoShape *shape() const;

    /**
     * Returns the type of the anchor.
     *
     * The text:anchor-type attribute specifies how a frame is bound to a
     * text document. The anchor position is the point at which a frame is
     * bound to a text document. The defined values for the text:anchor-type
     * attribute are;
     *
     * - as-char
     *   There is no anchor position. The drawing shape behaves like a
     *   character.
     * - char
     *   The character after the drawing shape element.
     * - frame
     *   The parent text box that the current drawing shape element is
     *   contained in.
     *  FIXME we don't support type frame
     * - page
     *   The page that has the same physical page number as the value of the
     *   text:anchor-page-number attribute that is attached to the drawing
     *   shape element.
     * - paragraph
     *   The paragraph that the current drawing shape element is contained in.
     */
    AnchorType anchorType() const;

    /**
     * Set how the anchor behaves
     */
    void setAnchorType(AnchorType type);

    /// set the current vertical-pos
    void setHorizontalPos(HorizontalPos);

    /// return the current vertical-pos
    HorizontalPos horizontalPos() const;

    /// set the current vertical-rel
    void setHorizontalRel(HorizontalRel);

    /// return the current vertical-rel
    HorizontalRel horizontalRel() const;

    /// set the current horizontal-pos
    void setVerticalPos(VerticalPos);

    /// return the current horizontal-pos
    VerticalPos verticalPos() const;

    /// set the current horizontal-rel
    void setVerticalRel(VerticalRel);

    /// return the current horizontal-rel
    VerticalRel verticalRel() const;

    /// return the wrap influence on position
    QString wrapInfluenceOnPosition() const;

    /// return if flow-with-text (odf attribute)
    bool flowWithText() const;

    /// return the page number of the shape (valid with page anchoring, -1 indicates auto).
    int pageNumber() const;

    /// return the offset of the shape from the anchor.
    const QPointF &offset() const;

    /// set the new offset of the shape. Causes a new layout soon.
    void setOffset(const QPointF &offset);

    /// Get extra data structure that is what is actually inside a text document
    TextLocation *textLocation() const;

    /// Set extra data structure that is what is actually inside a text document
    /// We do NOT take ownership (may change in the future)
    void setTextLocation(TextLocation *textLocation);

    /// Get placement strategy which is used to position shape linked to text anchor
    PlacementStrategy *placementStrategy() const;

    /// Set placement strategy which is used to position shape linked to text anchor
    /// We take owner ship and will make sure the strategy is deleted
    void setPlacementStrategy(PlacementStrategy *placementStrategy);

private:
    class Private;
    Private * const d;
};

#endif
