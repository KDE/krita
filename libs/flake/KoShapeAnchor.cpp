/* This file is part of the KDE project
 * Copyright (C) 2007, 2009-2010 Thomas Zander <zander@kde.org>
 * Copyright (C) 2010 Ko Gmbh <cbo@kogmbh.com>
 * Copyright (C) 2011 Matus Hanzes <matus.hanzes@ixonos.com>
 * Copyright (C) 2013 C. Boemann <cbo@boemann.dk>
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

#include "KoShapeAnchor.h"
#include "KoStyleStack.h"
#include "KoOdfLoadingContext.h"

#include <KoShapeContainer.h>
#include <KoXmlWriter.h>
#include <KoXmlReader.h>
#include <KoXmlNS.h>
#include <KoShapeSavingContext.h>
#include <KoShapeLoadingContext.h>

#include <QRectF>
#include <QTransform>
#include <FlakeDebug.h>

#include <KoGenChanges.h>

class Q_DECL_HIDDEN KoShapeAnchor::Private
{
public:
    Private(KoShape *s)
            : shape(s)
            , verticalPos(KoShapeAnchor::VTop)
            , verticalRel(KoShapeAnchor::VLine)
            , horizontalPos(KoShapeAnchor::HLeft)
            , horizontalRel(KoShapeAnchor::HChar)
            , flowWithText(true)
            , anchorType(KoShapeAnchor::AnchorToCharacter)
            , placementStrategy(0)
            , pageNumber(-1)
            , textLocation(0)
    {
    }


    QDebug printDebug(QDebug dbg) const
    {
#ifndef NDEBUG
        dbg.space() << "KoShapeAnchor" << this;
        dbg.space() << "offset:" << offset;
        dbg.space() << "shape:" << shape->name();
#endif
        return dbg.space();
    }

    KoShape * const shape;
    QPointF offset;
    KoShapeAnchor::VerticalPos verticalPos;
    KoShapeAnchor::VerticalRel verticalRel;
    KoShapeAnchor::HorizontalPos horizontalPos;
    KoShapeAnchor::HorizontalRel horizontalRel;
    QString wrapInfluenceOnPosition;
    bool flowWithText;
    KoShapeAnchor::AnchorType anchorType;
    KoShapeAnchor::PlacementStrategy *placementStrategy;
    int pageNumber;
    KoShapeAnchor::TextLocation *textLocation;
};

KoShapeAnchor::KoShapeAnchor(KoShape *shape)
    : d(new Private(shape))
{
}

KoShapeAnchor::~KoShapeAnchor()
{
    if (d->placementStrategy != 0) {
        delete d->placementStrategy;
    }
    delete d;
}

KoShape *KoShapeAnchor::shape() const
{
    return d->shape;
}

KoShapeAnchor::AnchorType KoShapeAnchor::anchorType() const
{
    return d->anchorType;
}

void KoShapeAnchor::setHorizontalPos(HorizontalPos hp)
{
    d->horizontalPos = hp;
}

KoShapeAnchor::HorizontalPos KoShapeAnchor::horizontalPos() const
{
    return d->horizontalPos;
}

void KoShapeAnchor::setHorizontalRel(HorizontalRel hr)
{
    d->horizontalRel = hr;
}

KoShapeAnchor::HorizontalRel KoShapeAnchor::horizontalRel() const
{
    return d->horizontalRel;
}

void KoShapeAnchor::setVerticalPos(VerticalPos vp)
{
    d->verticalPos = vp;
}

KoShapeAnchor::VerticalPos KoShapeAnchor::verticalPos() const
{
    return d->verticalPos;
}

void KoShapeAnchor::setVerticalRel(VerticalRel vr)
{
    d->verticalRel = vr;
}

KoShapeAnchor::VerticalRel KoShapeAnchor::verticalRel() const
{
    return d->verticalRel;
}

QString KoShapeAnchor::wrapInfluenceOnPosition() const
{
    return d->wrapInfluenceOnPosition;
}

bool KoShapeAnchor::flowWithText() const
{
    return d->flowWithText;
}

int KoShapeAnchor::pageNumber() const
{
    return d->pageNumber;
}

const QPointF &KoShapeAnchor::offset() const
{
    return d->offset;
}

void KoShapeAnchor::setOffset(const QPointF &offset)
{
    d->offset = offset;
}

void KoShapeAnchor::saveOdf(KoShapeSavingContext &context) const
{
    // anchor-type
    switch (d->anchorType) {
    case AnchorToCharacter:
        shape()->setAdditionalAttribute("text:anchor-type", "char");
        break;
    case AnchorAsCharacter:
        shape()->setAdditionalAttribute("text:anchor-type", "as-char");
        break;
    case AnchorParagraph:
        shape()->setAdditionalAttribute("text:anchor-type", "paragraph");
        break;
    case AnchorPage:
        shape()->setAdditionalAttribute("text:anchor-type", "page");
        break;
    default:
        break;
    }

    // vertical-pos
    switch (d->verticalPos) {
    case VBelow:
        shape()->setAdditionalStyleAttribute("style:vertical-pos", "below");
        break;
    case VBottom:
        shape()->setAdditionalStyleAttribute("style:vertical-pos", "bottom");
        break;
    case VFromTop:
        shape()->setAdditionalStyleAttribute("style:vertical-pos", "from-top");
        break;
    case VMiddle:
        shape()->setAdditionalStyleAttribute("style:vertical-pos", "middle");
        break;
    case VTop:
        shape()->setAdditionalStyleAttribute("style:vertical-pos", "top");
        break;
    default:
        break;
    }

    // vertical-rel
    switch (d->verticalRel) {
    case VBaseline:
        shape()->setAdditionalStyleAttribute("style:vertical-rel", "baseline");
        break;
    case VChar:
        shape()->setAdditionalStyleAttribute("style:vertical-rel", "char");
        break;
    case VFrame:
        shape()->setAdditionalStyleAttribute("style:vertical-rel", "frame");
        break;
    case VFrameContent:
        shape()->setAdditionalStyleAttribute("style:vertical-rel", "frame-content");
        break;
    case VLine:
        shape()->setAdditionalStyleAttribute("style:vertical-rel", "line");
        break;
    case VPage:
        shape()->setAdditionalStyleAttribute("style:vertical-rel", "page");
        break;
    case VPageContent:
        shape()->setAdditionalStyleAttribute("style:vertical-rel", "page-content");
        break;
    case VParagraph:
        shape()->setAdditionalStyleAttribute("style:vertical-rel", "paragraph");
        break;
    case VParagraphContent:
        shape()->setAdditionalStyleAttribute("style:vertical-rel", "paragraph-content");
        break;
    case VText:
        shape()->setAdditionalStyleAttribute("style:vertical-rel", "text");
        break;
    default:
        break;
    }

    // horizontal-pos
    switch (d->horizontalPos) {
    case HCenter:
        shape()->setAdditionalStyleAttribute("style:horizontal-pos", "center");
        break;
    case HFromInside:
        shape()->setAdditionalStyleAttribute("style:horizontal-pos", "from-inside");
        break;
    case HFromLeft:
        shape()->setAdditionalStyleAttribute("style:horizontal-pos", "from-left");
        break;
    case HInside:
        shape()->setAdditionalStyleAttribute("style:horizontal-posl", "inside");
        break;
    case HLeft:
        shape()->setAdditionalStyleAttribute("style:horizontal-pos", "left");
        break;
    case HOutside:
        shape()->setAdditionalStyleAttribute("style:horizontal-pos", "outside");
        break;
    case HRight:
        shape()->setAdditionalStyleAttribute("style:horizontal-pos", "right");
        break;
    default:
        break;
    }

    // horizontal-rel
    switch (d->horizontalRel) {
    case HChar:
        shape()->setAdditionalStyleAttribute("style:horizontal-rel", "char");
        break;
    case HPage:
        shape()->setAdditionalStyleAttribute("style:horizontal-rel", "page");
        break;
    case HPageContent:
        shape()->setAdditionalStyleAttribute("style:horizontal-rel", "page-content");
        break;
    case HPageStartMargin:
        shape()->setAdditionalStyleAttribute("style:horizontal-rel", "page-start-margin");
        break;
    case HPageEndMargin:
        shape()->setAdditionalStyleAttribute("style:horizontal-rel", "page-end-margin");
        break;
    case HFrame:
        shape()->setAdditionalStyleAttribute("style:horizontal-rel", "frame");
        break;
    case HFrameContent:
        shape()->setAdditionalStyleAttribute("style:horizontal-rel", "frame-content");
        break;
    case HFrameEndMargin:
        shape()->setAdditionalStyleAttribute("style:horizontal-rel", "frame-end-margin");
        break;
    case HFrameStartMargin:
        shape()->setAdditionalStyleAttribute("style:horizontal-rel", "frame-start-margin");
        break;
    case HParagraph:
        shape()->setAdditionalStyleAttribute("style:horizontal-rel", "paragraph");
        break;
    case HParagraphContent:
        shape()->setAdditionalStyleAttribute("style:horizontal-rel", "paragraph-content");
        break;
    case HParagraphEndMargin:
        shape()->setAdditionalStyleAttribute("style:horizontal-rel", "paragraph-end-margin");
        break;
    case HParagraphStartMargin:
        shape()->setAdditionalStyleAttribute("style:horizontal-rel", "paragraph-start-margin");
        break;
    default:
        break;
    }

    if (!d->wrapInfluenceOnPosition.isEmpty()) {
        shape()->setAdditionalStyleAttribute("draw:wrap-influence-on-position", d->wrapInfluenceOnPosition);
    }

    if (d->flowWithText) {
        shape()->setAdditionalStyleAttribute("style:flow-with-text", "true");
    } else {
        shape()->setAdditionalStyleAttribute("style:flow-with-text", "false");
    }

    if (shape()->parent()) {// an anchor may not yet have been layout-ed
        QTransform parentMatrix = shape()->parent()->absoluteTransformation(0).inverted();
        QTransform shapeMatrix = shape()->absoluteTransformation(0);

        qreal dx = d->offset.x() - shapeMatrix.dx()*parentMatrix.m11()
                                   - shapeMatrix.dy()*parentMatrix.m21();
        qreal dy = d->offset.y() - shapeMatrix.dx()*parentMatrix.m12()
                                   - shapeMatrix.dy()*parentMatrix.m22();
        context.addShapeOffset(shape(), QTransform(parentMatrix.m11(),parentMatrix.m12(),
                                                parentMatrix.m21(),parentMatrix.m22(),
                                                dx,dy));
    }

    shape()->saveOdf(context);

    context.removeShapeOffset(shape());
}

bool KoShapeAnchor::loadOdf(const KoXmlElement &element, KoShapeLoadingContext &context)
{
    d->offset = shape()->position();

    QString anchorType = shape()->additionalAttribute("text:anchor-type");

    if (anchorType == "char") {
        d->anchorType = AnchorToCharacter;
    } else if (anchorType == "as-char") {
        d->anchorType = AnchorAsCharacter;
        d->horizontalRel = HChar;
        d->horizontalPos = HLeft;
    } else if (anchorType == "paragraph") {
        d->anchorType = AnchorParagraph;
    } else {
        d->anchorType = AnchorPage;
        // it has different defaults at least LO thinks so - ODF doesn't define defaults for this
        d->horizontalPos = HFromLeft;
        d->verticalPos = VFromTop;
        d->horizontalRel = HPage;
        d->verticalRel = VPage;
    }

    if (anchorType == "page" && shape()->hasAdditionalAttribute("text:anchor-page-number")) {
        d->pageNumber = shape()->additionalAttribute("text:anchor-page-number").toInt();
        if (d->pageNumber <= 0) {
            // invalid if the page-number is invalid (OO.org does the same)
            // see http://bugs.kde.org/show_bug.cgi?id=281869
            d->pageNumber = -1;
        }
    } else {
        d->pageNumber = -1;
    }
    // always make it invisible or it will create empty rects on the first page
    // during initial layout. This is because only when we layout it's final page is
    // the shape moved away from page 1
    // in KWRootAreaProvider of textlayout it's set back to visible
    shape()->setVisible(false);

    // load settings from graphic style
    KoStyleStack &styleStack = context.odfLoadingContext().styleStack();
    styleStack.save();
    if (element.hasAttributeNS(KoXmlNS::draw, "style-name")) {
        context.odfLoadingContext().fillStyleStack(element, KoXmlNS::draw, "style-name", "graphic");
        styleStack.setTypeProperties("graphic");
    }
    QString verticalPos = styleStack.property(KoXmlNS::style, "vertical-pos");
    QString verticalRel = styleStack.property(KoXmlNS::style, "vertical-rel");
    QString horizontalPos = styleStack.property(KoXmlNS::style, "horizontal-pos");
    QString horizontalRel = styleStack.property(KoXmlNS::style, "horizontal-rel");
    d->wrapInfluenceOnPosition = styleStack.property(KoXmlNS::draw, "wrap-influence-on-position");
    QString flowWithText = styleStack.property(KoXmlNS::style, "flow-with-text");
    d->flowWithText = flowWithText.isEmpty() ? false : flowWithText == "true";
    styleStack.restore();

    // vertical-pos
    if (verticalPos == "below") {//svg:y attribute is ignored
        d->verticalPos = VBelow;
        d->offset.setY(0);
    } else if (verticalPos == "bottom") {//svg:y attribute is ignored
        d->verticalPos = VBottom;
        d->offset.setY(-shape()->size().height());
    } else if (verticalPos == "from-top") {
        d->verticalPos = VFromTop;
    } else if (verticalPos == "middle") {//svg:y attribute is ignored
        d->verticalPos = VMiddle;
        d->offset.setY(-(shape()->size().height()/2));
    } else if (verticalPos == "top") {//svg:y attribute is ignored
        d->verticalPos = VTop;
        d->offset.setY(0);
    }

    // vertical-rel
    if (verticalRel == "baseline")
        d->verticalRel = VBaseline;
    else if (verticalRel == "char")
        d->verticalRel = VChar;
    else if (verticalRel == "frame")
        d->verticalRel = VFrame;
    else if (verticalRel == "frame-content")
        d->verticalRel = VFrameContent;
    else if (verticalRel == "line")
        d->verticalRel = VLine;
    else if (verticalRel == "page")
        d->verticalRel = VPage;
    else if (verticalRel == "page-content")
        d->verticalRel = VPageContent;
    else if (verticalRel == "paragraph")
        d->verticalRel = VParagraph;
    else if (verticalRel == "paragraph-content")
        d->verticalRel = VParagraphContent;
    else if (verticalRel == "text")
        d->verticalRel = VText;

    // horizontal-pos
    if (horizontalPos == "center") {//svg:x attribute is ignored
        d->horizontalPos = HCenter;
        d->offset.setX(-(shape()->size().width()/2));
    } else if (horizontalPos == "from-inside") {
        d->horizontalPos = HFromInside;
    } else if (horizontalPos == "from-left") {
        d->horizontalPos = HFromLeft;
    } else if (horizontalPos == "inside") {//svg:x attribute is ignored
        d->horizontalPos = HInside;
        d->offset.setX(0);
    } else if (horizontalPos == "left") {//svg:x attribute is ignored
        d->horizontalPos = HLeft;
        d->offset.setX(0);
    }else if (horizontalPos == "outside") {//svg:x attribute is ignored
        d->horizontalPos = HOutside;
        d->offset.setX(-shape()->size().width());
    }else if (horizontalPos == "right") {//svg:x attribute is ignored
        d->horizontalPos = HRight;
        d->offset.setX(-shape()->size().width());
    }

    // horizontal-rel
    if (horizontalRel == "char")
        d->horizontalRel = HChar;
    else if (horizontalRel == "page")
        d->horizontalRel = HPage;
    else if (horizontalRel == "page-content")
        d->horizontalRel = HPageContent;
    else if (horizontalRel == "page-start-margin")
        d->horizontalRel = HPageStartMargin;
    else if (horizontalRel == "page-end-margin")
        d->horizontalRel = HPageEndMargin;
    else if (horizontalRel == "frame")
        d->horizontalRel = HFrame;
    else if (horizontalRel == "frame-content")
        d->horizontalRel = HFrameContent;
    else if (horizontalRel == "frame-end-margin")
        d->horizontalRel = HFrameEndMargin;
    else if (horizontalRel == "frame-start-margin")
        d->horizontalRel = HFrameStartMargin;
    else if (horizontalRel == "paragraph")
        d->horizontalRel = HParagraph;
    else if (horizontalRel == "paragraph-content")
        d->horizontalRel = HParagraphContent;
    else if (horizontalRel == "paragraph-end-margin")
        d->horizontalRel = HParagraphEndMargin;
    else if (horizontalRel == "paragraph-start-margin")
        d->horizontalRel = HParagraphStartMargin;

    // if svg:x or svg:y should be ignored set new position
    shape()->setPosition(d->offset);

    return true;
}

void KoShapeAnchor::setAnchorType(KoShapeAnchor::AnchorType type)
{
    d->anchorType = type;
    if (type == AnchorAsCharacter) {
        d->horizontalRel = HChar;
        d->horizontalPos = HLeft;
    }
}

KoShapeAnchor::TextLocation *KoShapeAnchor::textLocation() const
{
    return d->textLocation;
}

void KoShapeAnchor::setTextLocation(TextLocation *textLocation)
{
    d->textLocation = textLocation;
}

KoShapeAnchor::PlacementStrategy *KoShapeAnchor::placementStrategy() const
{
    return d->placementStrategy;
}

void KoShapeAnchor::setPlacementStrategy(PlacementStrategy *placementStrategy)
{
    if (placementStrategy != d->placementStrategy) {
        delete d->placementStrategy;

        d->placementStrategy = placementStrategy;
    }
}
