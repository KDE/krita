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
#include "TextShapeLayout.h"

#include <KoShapeContainer.h>
#include <KoTextShapeData.h>
#include <kdebug.h>

FloatingAnchorStrategy::FloatingAnchorStrategy(KoTextAnchor *anchor)
        : KoAnchorStrategy(),
          m_anchor(anchor),
          m_knowledgePoint(-1),
          m_finished(false),
          m_relayoutNeeded(false),
          m_relayoutPosition(0,0)
{
    calculateKnowledgePoint();
}

FloatingAnchorStrategy::~FloatingAnchorStrategy()
{
}

bool FloatingAnchorStrategy::positionShape(KoTextDocumentLayout::LayoutState *state)
{
    if (m_finished) { // shape is in right position no second pass needed
        return false;
    }

    if (!m_anchor->shape()->parent()) {
        return false;
    }

    // get the page data
    KoTextShapeData *data = qobject_cast<KoTextShapeData*>(m_anchor->shape()->parent()->userData());
    if (!data) {
        return false;
    }

    if (m_knowledgePoint < 0 ) {
        calculateKnowledgePoint();
        return false;
    }

    if (m_knowledgePoint > state->cursorPosition()) {
        return false;
    }

    // there should be always at least one line
    if (state->layout->lineCount() == 0) {
        return false;
    }

    QTextBlock block = m_anchor->document()->findBlock(m_anchor->positionInDocument());
    QTextLayout *layout = block.layout();

    QRectF boundingRect = m_anchor->shape()->boundingRect();
    QRectF containerBoundingRect = m_anchor->shape()->parent()->boundingRect();
    QRectF anchorBoundingRect;
    QPointF newPosition;

    // set anchor bounding rectangle horizontal position and size
    if (!countHorizontalRel(anchorBoundingRect, containerBoundingRect, state, block, layout)) {
        return false;
    }

    // set anchor bounding rectangle vertical position
    if (!countVerticalRel(anchorBoundingRect, containerBoundingRect, data, block, layout)) {
        return false;
    }

    // Set shape horizontal alignment inside anchor bounding rectangle
    countHorizontalPos(newPosition, anchorBoundingRect, containerBoundingRect);

    // Set shape vertical alignment inside anchor bounding rectangle
    countVerticalPos(newPosition, anchorBoundingRect, containerBoundingRect);

    newPosition = newPosition + m_anchor->offset();

    //check the border of page an move the shape back to have it visible
    checkPageBorder(newPosition, containerBoundingRect);

    // set the shape to the proper position based on the data
    m_anchor->shape()->update();
    m_anchor->shape()->setPosition(newPosition);
    m_anchor->shape()->update();

    // check if the shape is intersecting the text if no than no relayout is needed
    QRectF shapeRect(newPosition.x(),newPosition.y(), m_anchor->shape()->size().width(),m_anchor->shape()->size().height());
    m_relayoutPosition.setX(containerBoundingRect.y() + containerBoundingRect.height());

    if (checkTextIntersecion(m_relayoutPosition, shapeRect, containerBoundingRect, state, data) == false) {
        m_relayoutNeeded = false;
        m_finished = true;
        return true;
    }

    m_relayoutNeeded = true;
    m_finished = true;
    return true;
}

bool FloatingAnchorStrategy::isPositioned()
{
    return m_finished;
}

void FloatingAnchorStrategy::reset()
{
    calculateKnowledgePoint();
    m_finished = false;
    m_relayoutNeeded = false;
    return;
}

bool FloatingAnchorStrategy::isRelayoutNeeded()
{
    return m_relayoutNeeded;
}

QPointF FloatingAnchorStrategy::relayoutPosition()
{
    return m_relayoutPosition;
}

void FloatingAnchorStrategy::calculateKnowledgePoint()
{
    m_knowledgePoint = -1;

   // figure out until what cursor position we need to layout to get all the info we need
   switch (m_anchor->horizontalRel()) {
   case KoTextAnchor::HPage:
   case KoTextAnchor::HPageContent:
   case KoTextAnchor::HParagraph:
   case KoTextAnchor::HParagraphContent:
   case KoTextAnchor::HChar:
   case KoTextAnchor::HPageEndMargin:
   case KoTextAnchor::HPageStartMargin:
   case KoTextAnchor::HParagraphEndMargin:
   case KoTextAnchor::HParagraphStartMargin:
       m_knowledgePoint = m_anchor->positionInDocument();
       break;
   default :
       kDebug(32002) << "horizontal-rel not handled";
   }

   switch (m_anchor->verticalRel()) {
   case KoTextAnchor::VPage:
   case KoTextAnchor::VPageContent: {
       if (m_anchor->shape()->parent() == 0) // not enough info yet.
           return;
       KoTextShapeData *data = qobject_cast<KoTextShapeData*>(m_anchor->shape()->parent()->userData());
       Q_ASSERT(data);
       m_knowledgePoint = qMax(m_knowledgePoint, data->position() + 1);
       break;
   }
   case KoTextAnchor::VParagraphContent:
   case KoTextAnchor::VParagraph:
   case KoTextAnchor::VLine: {
       QTextBlock block = m_anchor->document()->findBlock(m_anchor->positionInDocument());
       m_knowledgePoint = qMax(m_knowledgePoint, block.position() + block.length()-2);
       break;
   }
   default :
       kDebug(32002) << "vertical-rel not handled";
   }
}

bool FloatingAnchorStrategy::countHorizontalRel(QRectF &anchorBoundingRect, QRectF containerBoundingRect,
                                          KoTextDocumentLayout::LayoutState *state, QTextBlock &block, QTextLayout *layout)
{
    switch (m_anchor->horizontalRel()) {
     case KoTextAnchor::HPage:
         anchorBoundingRect.setX(m_anchor->pageRect().x());
         anchorBoundingRect.setWidth(m_anchor->pageRect().width());
         break;

     case KoTextAnchor::HParagraph:
     case KoTextAnchor::HPageContent:
         anchorBoundingRect.setX(containerBoundingRect.x());
         anchorBoundingRect.setWidth(containerBoundingRect.width());
         break;

     case KoTextAnchor::HParagraphContent:
         anchorBoundingRect.setX(state->x() + containerBoundingRect.x());
         anchorBoundingRect.setWidth(state->width());
         break;

     case KoTextAnchor::HChar:
         if (layout->lineCount() != 0) {
             QTextLine tl = layout->lineForTextPosition(m_anchor->positionInDocument() - block.position());
             if (tl.isValid()) {
                 anchorBoundingRect.setX(tl.cursorToX(m_anchor->positionInDocument() - block.position()) + containerBoundingRect.x());
                 anchorBoundingRect.setWidth(0.1); // just some small value
             } else {
                 return false; // lets go for a second round.
             }
         } else {
             return false; // lets go for a second round.
         }
         break;

     case KoTextAnchor::HPageStartMargin:
     {
         int horizontalPos = m_anchor->horizontalPos();
         // if verticalRel is HFromInside or HInside or HOutside and the page number is even,
         // than set anchorBoundingRect to HPageEndMargin area
         if ((m_anchor->pageNumber()%2 == 0) && (horizontalPos == KoTextAnchor::HFromInside ||
                 horizontalPos == KoTextAnchor::HInside || horizontalPos == KoTextAnchor::HOutside)) {
             anchorBoundingRect.setX(containerBoundingRect.x() + containerBoundingRect.width());
             anchorBoundingRect.setWidth(m_anchor->pageRect().width() - anchorBoundingRect.x());
         } else {
             anchorBoundingRect.setX(m_anchor->pageRect().x());
             anchorBoundingRect.setWidth(containerBoundingRect.x());
         }
         break;
     }
     case KoTextAnchor::HPageEndMargin:
     {
         int horizontalPos = m_anchor->horizontalPos();
         // if verticalRel is HFromInside or HInside or HOutside and the page number is even,
         // than set anchorBoundingRect to HPageStartMargin area
         if ((m_anchor->pageNumber()%2 == 0) && (horizontalPos == KoTextAnchor::HFromInside ||
                 horizontalPos == KoTextAnchor::HInside || horizontalPos == KoTextAnchor::HOutside)) {
             anchorBoundingRect.setX(m_anchor->pageRect().x());
             anchorBoundingRect.setWidth(containerBoundingRect.x());
         } else {
             anchorBoundingRect.setX(containerBoundingRect.x() + containerBoundingRect.width());
             anchorBoundingRect.setWidth(m_anchor->pageRect().width() - anchorBoundingRect.x());
         }
         break;
     }
     case KoTextAnchor::HParagraphStartMargin:
     {
         int horizontalPos = m_anchor->horizontalPos();
         // if verticalRel is HFromInside or HInside or HOutside and the page number is even,
         // than set anchorBoundingRect to HParagraphEndMargin area
         if ((m_anchor->pageNumber()%2 == 0) && (horizontalPos == KoTextAnchor::HFromInside ||
                 horizontalPos == KoTextAnchor::HInside || horizontalPos == KoTextAnchor::HOutside)) {
             anchorBoundingRect.setX(state->x() + containerBoundingRect.x() + state->width());
             anchorBoundingRect.setWidth(containerBoundingRect.x() + containerBoundingRect.width() - anchorBoundingRect.x());
         } else {
             anchorBoundingRect.setX(containerBoundingRect.x());
             anchorBoundingRect.setWidth(state->x());
         }
         break;
     }
     case KoTextAnchor::HParagraphEndMargin:
     {
         int horizontalPos = m_anchor->horizontalPos();
         // if verticalRel is HFromInside or HInside or HOutside and the page number is even,
         // than set anchorBoundingRect to HParagraphStartMargin area
         if ((m_anchor->pageNumber()%2 == 0) && (horizontalPos == KoTextAnchor::HFromInside ||
                 horizontalPos == KoTextAnchor::HInside || horizontalPos == KoTextAnchor::HOutside)) {
             anchorBoundingRect.setX(containerBoundingRect.x());
             anchorBoundingRect.setWidth(state->x());
         } else {
             anchorBoundingRect.setX(state->x() + containerBoundingRect.x() + state->width());
             anchorBoundingRect.setWidth(containerBoundingRect.x() + containerBoundingRect.width() - anchorBoundingRect.x());
         }
         break;
     }
     default :
         kDebug(32002) << "horizontal-rel not handled";
     }
    return true;
}

void FloatingAnchorStrategy::countHorizontalPos(QPointF &newPosition, QRectF anchorBoundingRect, QRectF containerBoundingRect)
{
    switch (m_anchor->horizontalPos()) {
    case KoTextAnchor::HCenter:
        newPosition.setX(anchorBoundingRect.x() + anchorBoundingRect.width()/2 - containerBoundingRect.x());
        break;

    case KoTextAnchor::HFromInside:
    case KoTextAnchor::HInside:
    {
        if (m_anchor->pageNumber()%2 == 1) {
            newPosition.setX(anchorBoundingRect.x() - containerBoundingRect.x());
        } else {
            newPosition.setX(anchorBoundingRect.right() - containerBoundingRect.x() -
                    m_anchor->shape()->size().width() - 2*m_anchor->offset().x() );
        }
        break;
    }
    case KoTextAnchor::HFromLeft:
    case KoTextAnchor::HLeft:
        newPosition.setX(anchorBoundingRect.x() - containerBoundingRect.x());
        break;

    case KoTextAnchor::HOutside:
    {
        if (m_anchor->pageNumber()%2 == 1) {
            newPosition.setX(anchorBoundingRect.right() - containerBoundingRect.x());
        } else {
            newPosition.setX(anchorBoundingRect.x() - containerBoundingRect.x() +
                             m_anchor->shape()->size().width() - 2*(m_anchor->offset().x() + m_anchor->shape()->size().width()) );
        }
        break;
    }
    case KoTextAnchor::HRight: {
        newPosition.setX(anchorBoundingRect.right() - containerBoundingRect.x());
        break;
    }
    default :
        kDebug(32002) << "horizontal-pos not handled";
    }
}

bool FloatingAnchorStrategy::countVerticalRel(QRectF &anchorBoundingRect, QRectF containerBoundingRect,
                                          KoTextShapeData *data, QTextBlock &block, QTextLayout *layout)
{
    switch (m_anchor->verticalRel()) {
    case KoTextAnchor::VPage:
     anchorBoundingRect.setY(m_anchor->pageRect().y());
     anchorBoundingRect.setHeight(m_anchor->pageRect().height());
     break;

    case KoTextAnchor::VPageContent:
     anchorBoundingRect.setY(m_anchor->pageContentRect().y());
     anchorBoundingRect.setHeight(m_anchor->pageContentRect().height());
     break;

    case KoTextAnchor::VParagraph:
    case KoTextAnchor::VParagraphContent:
     if (layout->lineCount() != 0) {
         qreal top = layout->lineAt(0).y();
         QTextLine tl = layout->lineAt(layout->lineCount() - 1);
         anchorBoundingRect.setY(top + containerBoundingRect.y()  - data->documentOffset());
         anchorBoundingRect.setHeight(tl.y() + tl.height() - top);
         KoTextBlockData *blockData = dynamic_cast<KoTextBlockData*>(block.userData());
         if(blockData && m_anchor->verticalRel() == KoTextAnchor::VParagraph) {
             anchorBoundingRect.setY(blockData->effectiveTop() + containerBoundingRect.y()  - data->documentOffset());
         }
     } else {
         return false; // lets go for a second round.
     }
     break;

    case KoTextAnchor::VLine:
     if (layout->lineCount()) {
         QTextLine tl = layout->lineForTextPosition(m_anchor->positionInDocument() - block.position());
         Q_ASSERT(tl.isValid());
         anchorBoundingRect.setY(tl.y() - m_anchor->shape()->size().height()
                         + containerBoundingRect.y() - data->documentOffset());
         anchorBoundingRect.setHeight(2*m_anchor->shape()->size().height());
     } else {
         return false; // lets go for a second round.
     }
     break;

    case KoTextAnchor::VText: // same as char apparently only used when as-char
    case KoTextAnchor::VChar:
     if (layout->lineCount()) {
         QTextLine tl = layout->lineForTextPosition(m_anchor->positionInDocument() - block.position());
         Q_ASSERT(tl.isValid());
         if (m_anchor->behavesAsCharacter() && m_anchor->verticalRel() == KoTextAnchor::VChar) {
             //char relative is behaving in a special way when as-char
             anchorBoundingRect.setY(tl.y() + containerBoundingRect.y() - data->documentOffset());
             anchorBoundingRect.setHeight(tl.height());
         } else {
             anchorBoundingRect.setY(tl.y() + containerBoundingRect.y() - data->documentOffset());
             anchorBoundingRect.setHeight(tl.height());
         }
     } else {
         return false; // lets go for a second round.
     }
     break;

    case KoTextAnchor::VBaseline:
     if (layout->lineCount()) {
         QTextLine tl = layout->lineForTextPosition(m_anchor->positionInDocument() - block.position());
         Q_ASSERT(tl.isValid());
         anchorBoundingRect.setY(tl.y() + tl.ascent() - m_anchor->shape()->size().height()
            + containerBoundingRect.y() - data->documentOffset());
         anchorBoundingRect.setHeight(2*m_anchor->shape()->size().height());
     } else {
         return false; // lets go for a second round.
     }
     break;
    default :
     kDebug(32002) << "vertical-rel not handled";
    }
    return true;
}

void FloatingAnchorStrategy::countVerticalPos(QPointF &newPosition, QRectF anchorBoundingRect, QRectF containerBoundingRect)
{
    switch (m_anchor->verticalPos()) {
    case KoTextAnchor::VBottom:
        newPosition.setY(anchorBoundingRect.bottom() - containerBoundingRect.y()
        );//- m_anchor->shape()->size().height());
        break;
    case KoTextAnchor::VBelow:
        newPosition.setY(anchorBoundingRect.bottom() - containerBoundingRect.y());
        break;

    case KoTextAnchor::VMiddle:
        newPosition.setY(anchorBoundingRect.y() + anchorBoundingRect.height()/2 - containerBoundingRect.y());
        break;

    case KoTextAnchor::VFromTop:
    case KoTextAnchor::VTop:
        newPosition.setY(anchorBoundingRect.y() - containerBoundingRect.y());
        break;

    default :
        kDebug(32002) << "vertical-pos not handled";
    }

}

void FloatingAnchorStrategy::checkPageBorder(QPointF &newPosition, QRectF containerBoundingRect)
{
    //check left border and move the shape back to have the whole shape visible
    if (newPosition.x() < m_anchor->pageRect().x() - containerBoundingRect.x()) {
        newPosition.setX(m_anchor->pageRect().x() - containerBoundingRect.x());
    }

    //check right border and move the shape back to have the whole shape visible
    if ((newPosition.x() + m_anchor->shape()->size().width()) > (m_anchor->pageRect().x() + m_anchor->pageRect().width() - containerBoundingRect.x())) {
        newPosition.setX(m_anchor->pageRect().x() + m_anchor->pageRect().width() - m_anchor->shape()->size().width() - containerBoundingRect.x());
    }

    //check top border and move the shape back to have the whole shape visible
    if (newPosition.y() < (m_anchor->pageRect().y() - containerBoundingRect.y())) {
        newPosition.setY(m_anchor->pageRect().y() - containerBoundingRect.y());
    }

    //check bottom border and move the shape back to have the whole shape visible
    if ((newPosition.y() + m_anchor->shape()->size().height()) > (m_anchor->pageRect().y() + m_anchor->pageRect().height() - containerBoundingRect.y())) {
        newPosition.setY(m_anchor->pageRect().y() + m_anchor->pageRect().height() - m_anchor->shape()->size().height() - containerBoundingRect.y());
    }
}

bool FloatingAnchorStrategy::checkTextIntersecion(QPointF &relayoutPos, QRectF shpRect, QRectF contRect, KoTextDocumentLayout::LayoutState *state,
                                            KoTextShapeData *data)
{
    bool intersectionX = false;
    bool intersectionY = false;

    //check text area
    QTextLayout *layout = state->layout;
    QTextLine tl = layout->lineAt(layout->lineCount() - 1);

    qreal shpRight = shpRect.left() + shpRect.width();
    qreal contRight = contRect.width();
    qreal shpBottom = shpRect.top() + shpRect.height();
    qreal contBottom = tl.y() + tl.height() - data->documentOffset();

    //horizontal
    if (((shpRect.left() > 0) && (shpRight > 0) && (shpRect.left() < contRight)) ||
        ((shpRight > 0) && (shpRect.left() < contRight) && (shpRight < contRight)) ||
        ((shpRect.left() < 0) && (shpRect.right() > contRight))) {
        if (shpRect.left() < 0) {
            relayoutPos.setX(0);
        } else {
            relayoutPos.setX(shpRect.left());
        }
        intersectionX = true;
    }

    //vertical
    if (((shpRect.top() > 0) && (shpBottom > 0) && (shpRect.top() < contBottom)) ||
        ((shpBottom > 0) && (shpRect.top() < contBottom) && (shpBottom < contBottom)) ||
        ((shpRect.top() < 0) && (shpBottom > contBottom))) {
        if (shpRect.top() < 0) {
            relayoutPos.setY(0);
        } else {
            relayoutPos.setY(shpRect.top());
        }
        intersectionY = true;
    }

    if (intersectionX && intersectionY) {
        return true;
    }

    return false;
}
