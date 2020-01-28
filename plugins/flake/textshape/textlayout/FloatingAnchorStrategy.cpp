/* This file is part of the KDE project
 * Copyright (C) 2007, 2009, 2010 Thomas Zander <zander@kde.org>
 * Copyright (C) 2011 Matus Hanzes <matus.hanzes@ixonos.com>
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

#include "FloatingAnchorStrategy.h"

#include "KoTextDocumentLayout.h"
#include "KoTextLayoutObstruction.h"

#include <KoShapeContainer.h>
#include <KoTextShapeData.h>
#include <KoTextLayoutRootArea.h>
#include <KoAnchorTextRange.h>

#include <TextLayoutDebug.h>

#include <QTextLayout>
#include <QTextBlock>

FloatingAnchorStrategy::FloatingAnchorStrategy(KoAnchorTextRange *anchorRange, KoTextLayoutRootArea *rootArea)
    : AnchorStrategy(anchorRange->anchor(), rootArea)
    , m_obstruction(new KoTextLayoutObstruction(anchorRange->anchor()->shape(), QTransform()))
    , m_anchorRange(anchorRange)
{
}

FloatingAnchorStrategy::~FloatingAnchorStrategy()
{
}


void FloatingAnchorStrategy::updateObstruction(qreal documentOffset)
{
    KoTextDocumentLayout *layout = dynamic_cast<KoTextDocumentLayout *>(m_anchorRange->document()->documentLayout());

    QTransform matrix = m_anchor->shape()->absoluteTransformation();
    matrix = matrix * m_anchor->shape()->parent()->absoluteTransformation().inverted();
    matrix.translate(0, documentOffset);
    m_obstruction->changeMatrix(matrix);

    layout->registerAnchoredObstruction(m_obstruction);
}

//should return true while we are still moving around
bool FloatingAnchorStrategy::moveSubject()
{
    if (!m_anchor->shape()->parent()) {
        return false; // let's fake we moved to force another relayout
    }

    // get the page data
    KoTextShapeData *data = qobject_cast<KoTextShapeData*>(m_anchor->shape()->parent()->userData());
    if (!data) {
        return false; // let's fake we moved to force another relayout
    }

    QTextBlock block = m_anchorRange->document()->findBlock(m_anchorRange->position());
    QTextLayout *layout = block.layout();

    // there should be always at least one line
    if (layout->lineCount() == 0) {
        return false; // let's fake we moved to force another relayout
    }

    // The bounding rect of the textshape in document coords
    QRectF containerBoundingRect = m_anchor->shape()->parent()->boundingRect();

    // The (to be calculated) reference rect for anchoring in document coords
    QRectF anchorBoundingRect;

    // This is in coords relative to texshape
    QPointF newPosition;

    QPointF offset;
    if (m_anchor->horizontalPos() == KoShapeAnchor::HFromLeft
        || m_anchor->horizontalPos() == KoShapeAnchor::HFromInside) {
        offset.setX(m_anchor->offset().x());
    }
    if (m_anchor->verticalPos() == KoShapeAnchor::VFromTop) {
        offset.setY(m_anchor->offset().y());
    }

    // set anchor bounding rectangle horizontal position and size
    if (!countHorizontalRel(anchorBoundingRect, containerBoundingRect, block, layout)) {
        return false; // let's fake we moved to force another relayout
    }

    // set anchor bounding rectangle vertical position
    if (!countVerticalRel(anchorBoundingRect, containerBoundingRect, data, block, layout)) {
        return false; // let's fake we moved to force another relayout
    }

    // Set shape horizontal alignment inside anchor bounding rectangle
    countHorizontalPos(newPosition, anchorBoundingRect);

    // Set shape vertical alignment inside anchor bounding rectangle
    countVerticalPos(newPosition, anchorBoundingRect);

    newPosition += offset;

    //check the border of page and move the shape back to have it visible
    checkPageBorder(newPosition);

    newPosition -= containerBoundingRect.topLeft();

    //check the border of layout environment and move the shape back to have it within
    if (m_anchor->flowWithText()) {
        checkLayoutEnvironment(newPosition, data);
    }


    checkStacking(newPosition);

    if (newPosition == m_anchor->shape()->position()) {
        if (m_anchor->shape()->textRunAroundSide() != KoShape::RunThrough) {
            updateObstruction(data->documentOffset());
        }
        return true;
    }

    // set the shape to the proper position based on the data
    m_anchor->shape()->update();
    m_anchor->shape()->setPosition(newPosition);
    m_anchor->shape()->update();

    if (m_anchor->shape()->textRunAroundSide() != KoShape::RunThrough) {
        updateObstruction(data->documentOffset());
    }

    return true;
}

bool FloatingAnchorStrategy::countHorizontalRel(QRectF &anchorBoundingRect, const QRectF &containerBoundingRect, QTextBlock &block, QTextLayout *layout)
{
    switch (m_anchor->horizontalRel()) {
    case KoShapeAnchor::HPage:
        anchorBoundingRect.setX(pageRect().x());
        anchorBoundingRect.setWidth(pageRect().width());
        break;

    case KoShapeAnchor::HFrameContent:
    case KoShapeAnchor::HFrame:
        anchorBoundingRect.setX(containerBoundingRect.x());
        anchorBoundingRect.setWidth(containerBoundingRect.width());
        break;

    case KoShapeAnchor::HPageContent:
        anchorBoundingRect.setX(pageContentRect().x());
        anchorBoundingRect.setWidth(pageContentRect().width());
        break;

    case KoShapeAnchor::HParagraph:
        anchorBoundingRect.setX(paragraphRect().x() + containerBoundingRect.x());
        anchorBoundingRect.setWidth(paragraphRect().width());
        break;

    case KoShapeAnchor::HParagraphContent:
        anchorBoundingRect.setX(paragraphContentRect().x() + containerBoundingRect.x());
        anchorBoundingRect.setWidth(paragraphContentRect().width());
        break;

    case KoShapeAnchor::HChar: {
        QTextLine tl = layout->lineForTextPosition(m_anchorRange->position() - block.position());
        if (!tl.isValid())
            return false; // lets go for a second round.
        anchorBoundingRect.setX(tl.cursorToX(m_anchorRange->position() - block.position()) + containerBoundingRect.x());
        anchorBoundingRect.setWidth(0.1); // just some small value
        break;
    }
    case KoShapeAnchor::HPageStartMargin: {
        int horizontalPos = m_anchor->horizontalPos();
        // if verticalRel is HFromInside or HInside or HOutside and the page number is even,
        // than set anchorBoundingRect to HPageEndMargin area
        if ((pageNumber()%2 == 0) && (horizontalPos == KoShapeAnchor::HFromInside ||
                horizontalPos == KoShapeAnchor::HInside || horizontalPos == KoShapeAnchor::HOutside)) {
            anchorBoundingRect.setX(containerBoundingRect.x() + containerBoundingRect.width());
            anchorBoundingRect.setWidth(pageRect().width() - anchorBoundingRect.x());
        } else {
            anchorBoundingRect.setX(pageRect().x());
            anchorBoundingRect.setWidth(containerBoundingRect.x());
        }
        break;
    }
    case KoShapeAnchor::HPageEndMargin:
    {
        int horizontalPos = m_anchor->horizontalPos();
        // if verticalRel is HFromInside or HInside or HOutside and the page number is even,
        // than set anchorBoundingRect to HPageStartMargin area
        if ((pageNumber()%2 == 0) && (horizontalPos == KoShapeAnchor::HFromInside ||
                horizontalPos == KoShapeAnchor::HInside || horizontalPos == KoShapeAnchor::HOutside)) {
            anchorBoundingRect.setX(pageRect().x());
            anchorBoundingRect.setWidth(containerBoundingRect.x());
        } else {
            anchorBoundingRect.setX(containerBoundingRect.x() + containerBoundingRect.width());
            anchorBoundingRect.setWidth(pageRect().width() - anchorBoundingRect.x());
        }
        break;
    }
    case KoShapeAnchor::HParagraphStartMargin:
    {
        int horizontalPos = m_anchor->horizontalPos();
        // if verticalRel is HFromInside or HInside or HOutside and the page number is even,
        // than set anchorBoundingRect to HParagraphEndMargin area
        if ((pageNumber()%2 == 0) && (horizontalPos == KoShapeAnchor::HFromInside ||
                horizontalPos == KoShapeAnchor::HInside || horizontalPos == KoShapeAnchor::HOutside)) {
//FIXME             anchorBoundingRect.setX(state->x() + containerBoundingRect.x() + state->width());
            anchorBoundingRect.setWidth(containerBoundingRect.x() + containerBoundingRect.width() - anchorBoundingRect.x());
        } else {
            anchorBoundingRect.setX(containerBoundingRect.x());
//FIXME             anchorBoundingRect.setWidth(state->x());
        }
        break;
    }
    case KoShapeAnchor::HParagraphEndMargin:
    {
        int horizontalPos = m_anchor->horizontalPos();
        // if verticalRel is HFromInside or HInside or HOutside and the page number is even,
        // than set anchorBoundingRect to HParagraphStartMargin area
        if ((pageNumber()%2 == 0) && (horizontalPos == KoShapeAnchor::HFromInside ||
                horizontalPos == KoShapeAnchor::HInside || horizontalPos == KoShapeAnchor::HOutside)) {
            anchorBoundingRect.setX(containerBoundingRect.x());
//FIXME             anchorBoundingRect.setWidth(state->x());
        } else {
//FIXME             anchorBoundingRect.setX(state->x() + containerBoundingRect.x() + state->width());
            anchorBoundingRect.setWidth(containerBoundingRect.x() + containerBoundingRect.width() - anchorBoundingRect.x());
        }
        break;
    }
    default :
        warnTextLayout << "horizontal-rel not handled";
    }
    return true;
}

void FloatingAnchorStrategy::countHorizontalPos(QPointF &newPosition, const QRectF &anchorBoundingRect)
{
    switch (m_anchor->horizontalPos()) {
    case KoShapeAnchor::HCenter:
        newPosition.setX(anchorBoundingRect.x() + anchorBoundingRect.width()/2
         - m_anchor->shape()->size().width()/2);
        break;

    case KoShapeAnchor::HFromInside:
    case KoShapeAnchor::HInside:
    {
        if (pageNumber()%2 == 1) {
            newPosition.setX(anchorBoundingRect.x());
        } else {
            newPosition.setX(anchorBoundingRect.right() -
                    m_anchor->shape()->size().width() - 2*m_anchor->offset().x() );
        }
        break;
    }
    case KoShapeAnchor::HLeft:
    case KoShapeAnchor::HFromLeft:
        newPosition.setX(anchorBoundingRect.x());
        break;

    case KoShapeAnchor::HOutside:
    {
        if (pageNumber()%2 == 1) {
            newPosition.setX(anchorBoundingRect.right());
        } else {
            QSizeF size = m_anchor->shape()->boundingRect().size();
            newPosition.setX(anchorBoundingRect.x() - size.width() - m_anchor->offset().x());
        }
        break;
    }
    case KoShapeAnchor::HRight: {
        QSizeF size = m_anchor->shape()->boundingRect().size();
        newPosition.setX(anchorBoundingRect.right() - size.width());
        break;
    }
    default :
        warnTextLayout << "horizontal-pos not handled";
    }
}

bool FloatingAnchorStrategy::countVerticalRel(QRectF &anchorBoundingRect, const QRectF &containerBoundingRect,
                                          KoTextShapeData *data, QTextBlock &block, QTextLayout *layout)
{
    //FIXME proper handle VFrame and VFrameContent but fallback to VPage/VPageContent for now to produce better results

    switch (m_anchor->verticalRel()) {
    case KoShapeAnchor::VPage:
        anchorBoundingRect.setY(pageRect().y());
        anchorBoundingRect.setHeight(pageRect().height());
        break;

    case KoShapeAnchor::VFrame:
    case KoShapeAnchor::VFrameContent:
        anchorBoundingRect.setY(containerBoundingRect.y());
        anchorBoundingRect.setHeight(containerBoundingRect.height());
        break;

    case KoShapeAnchor::VPageContent:
        anchorBoundingRect.setY(pageContentRect().y());
        anchorBoundingRect.setHeight(pageContentRect().height());
        break;

    case KoShapeAnchor::VParagraph:
        anchorBoundingRect.setY(paragraphRect().y() + containerBoundingRect.y()  - data->documentOffset());
        anchorBoundingRect.setHeight(paragraphRect().height());
        break;

    case KoShapeAnchor::VParagraphContent: {
        anchorBoundingRect.setY(paragraphContentRect().y() + containerBoundingRect.y()  - data->documentOffset());
        anchorBoundingRect.setHeight(paragraphContentRect().height());
    }
    break;

    case KoShapeAnchor::VLine: {
        QTextLine tl = layout->lineForTextPosition(m_anchorRange->position() - block.position());
        if (!tl.isValid())
            return false; // lets go for a second round.
        QSizeF size = m_anchor->shape()->boundingRect().size();
        anchorBoundingRect.setY(tl.y() - size.height()
                        + containerBoundingRect.y() - data->documentOffset());
        anchorBoundingRect.setHeight(2*size.height());
    }
    break;

    case KoShapeAnchor::VText: // same as char apparently only used when as-char
    case KoShapeAnchor::VChar: {
         QTextLine tl = layout->lineForTextPosition(m_anchorRange->position() - block.position());
         if (!tl.isValid())
             return false; // lets go for a second round.
         anchorBoundingRect.setY(tl.y() + containerBoundingRect.y() - data->documentOffset());
         anchorBoundingRect.setHeight(tl.height());
     }
     break;

    case KoShapeAnchor::VBaseline: {
         QTextLine tl = layout->lineForTextPosition(m_anchorRange->position() - block.position());
         if (!tl.isValid())
             return false; // lets go for a second round.
         QSizeF size = m_anchor->shape()->boundingRect().size();
         anchorBoundingRect.setY(tl.y() + tl.ascent() - size.height()
            + containerBoundingRect.y() - data->documentOffset());
         anchorBoundingRect.setHeight(2*size.height());
     }
     break;
    default :
     warnTextLayout << "vertical-rel not handled";
    }
    return true;
}

void FloatingAnchorStrategy::countVerticalPos(QPointF &newPosition, const QRectF &anchorBoundingRect)
{
    switch (m_anchor->verticalPos()) {
    case KoShapeAnchor::VBottom:
        newPosition.setY(anchorBoundingRect.bottom() - m_anchor->shape()->size().height());
        break;
    case KoShapeAnchor::VBelow:
        newPosition.setY(anchorBoundingRect.bottom());
        break;

    case KoShapeAnchor::VMiddle:
        newPosition.setY(anchorBoundingRect.y() + anchorBoundingRect.height()/2 - m_anchor->shape()->size().height()/2);
        break;

    case KoShapeAnchor::VFromTop:
    case KoShapeAnchor::VTop:
        newPosition.setY(anchorBoundingRect.y());
        break;

    default :
        warnTextLayout << "vertical-pos not handled";
    }

}

void FloatingAnchorStrategy::checkLayoutEnvironment(QPointF &newPosition, KoTextShapeData *data)
{
    QSizeF size = m_anchor->shape()->boundingRect().size();

    //check left border and move the shape back to have the whole shape within
    if (newPosition.x() < layoutEnvironmentRect().x()) {
        newPosition.setX(layoutEnvironmentRect().x());
    }

    //check right border and move the shape back to have the whole shape within
    if (newPosition.x() + size.width() > layoutEnvironmentRect().right()) {
        newPosition.setX(layoutEnvironmentRect().right() - size.width());
    }

    //check top border and move the shape back to have the whole shape within
    if (newPosition.y() < layoutEnvironmentRect().y() - data->documentOffset()) {
        newPosition.setY(layoutEnvironmentRect().y() - data->documentOffset());
    }

    //check bottom border and move the shape back to have the whole shape within
    if (newPosition.y() + size.height() > layoutEnvironmentRect().bottom() - data->documentOffset()) {
        newPosition.setY(layoutEnvironmentRect().bottom() - size.height() - data->documentOffset());
    }
}

void FloatingAnchorStrategy::checkPageBorder(QPointF &newPosition)
{
    QSizeF size = m_anchor->shape()->boundingRect().size();

    //check left border and move the shape back to have the whole shape visible
    if (newPosition.x() < pageRect().x()) {
        newPosition.setX(pageRect().x());
    }

    //check right border and move the shape back to have the whole shape visible
    if (newPosition.x() + size.width() > pageRect().x() + pageRect().width()) {
        newPosition.setX(pageRect().x() + pageRect().width() - size.width());
    }

    //check top border and move the shape back to have the whole shape visible
    if (newPosition.y() < pageRect().y()) {
        newPosition.setY(pageRect().y());
    }

    //check bottom border and move the shape back to have the whole shape visible
    if (newPosition.y() + size.height() > pageRect().y() + pageRect().height()) {
        newPosition.setY(pageRect().y() + pageRect().height() - size.height());
    }
}

// If the horizontal-pos is Left or Right then we need to check if there are other
// objects anchored with horizontal-pos left or right. If there are then we need
// to "stack" our object on them what means that rather then floating this object
// over the other it is needed to adjust the position to be sure they are not
// floating over each other.
void FloatingAnchorStrategy::checkStacking(QPointF &newPosition)
{
    if (m_anchor->anchorType() != KoShapeAnchor::AnchorParagraph || (m_anchor->horizontalPos() != KoShapeAnchor::HLeft && m_anchor->horizontalPos() != KoShapeAnchor::HRight))
        return;

    int idx = m_rootArea->documentLayout()->textAnchors().indexOf(m_anchor);
    Q_ASSERT_X(idx >= 0, __FUNCTION__, QString("WTF? How can our anchor not be in the anchor-list but still be called?").toLocal8Bit());

    QSizeF size = m_anchor->shape()->boundingRect().size();
    for(int i = 0; i < idx; ++i) {
        KoShapeAnchor *a = m_rootArea->documentLayout()->textAnchors()[i];
        if (m_anchor->anchorType() != a->anchorType() || m_anchor->horizontalPos() != a->horizontalPos())
            continue;

        QRectF thisRect(newPosition, size);
        QRectF r(a->shape()->boundingRect());
        if (thisRect.intersects(r)) {
            if (m_anchor->horizontalPos() == KoShapeAnchor::HLeft)
                newPosition.setX(a->shape()->position().x() + r.width());
            else // KoShapeAnchor::HRight
                newPosition.setX(a->shape()->position().x() - size.width());
        }
    }
}
