/* This file is part of the KDE project
 * Copyright (C) 2011 C. Boemann <cbo@boemann.dk>
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

#include "SimpleRootAreaProvider.h"

#include "TextShape.h"

#include <KoBorder.h>
#include <KoTextLayoutRootArea.h>
#include <KoTextLayoutObstruction.h>

SimpleRootAreaProvider::SimpleRootAreaProvider(KoTextShapeData *data, TextShape *textshape)
    : m_textShape(textshape)
    , m_area(0)
    , m_textShapeData(data)
    , m_fixAutogrow(false)

{
}

KoTextLayoutRootArea *SimpleRootAreaProvider::provide(KoTextDocumentLayout *documentLayout, const RootAreaConstraint &, int requestedPosition, bool *isNewRootArea)
{
    if (m_area == 0) {
        *isNewRootArea = true;
        m_area = new KoTextLayoutRootArea(documentLayout);
        m_area->setAssociatedShape(m_textShape);
        m_textShapeData->setRootArea(m_area);

        return m_area;
    }
    if (requestedPosition == 0) {
        *isNewRootArea = false;
        return m_area;
    }
    return 0;
}

void SimpleRootAreaProvider::releaseAllAfter(KoTextLayoutRootArea *afterThis)
{
    Q_UNUSED(afterThis);
}

void SimpleRootAreaProvider::doPostLayout(KoTextLayoutRootArea *rootArea, bool isNewRootArea)
{
    Q_UNUSED(isNewRootArea);

    m_textShape->update(m_textShape->outlineRect());

    QSizeF newSize = m_textShape->size()
                     - QSizeF(m_textShapeData->leftPadding() + m_textShapeData->rightPadding(),
                              m_textShapeData->topPadding() + m_textShapeData->bottomPadding());

    KoBorder *border = m_textShape->border();

    if (border) {
        newSize -= QSizeF(border->borderWidth(KoBorder::LeftBorder) + border->borderWidth(KoBorder::RightBorder), border->borderWidth(KoBorder::TopBorder) + border->borderWidth(KoBorder::BottomBorder));
    }

    if (m_textShapeData->verticalAlignment() & Qt::AlignBottom) {
        // Do nothing
    }
    if (m_textShapeData->verticalAlignment() & Qt::AlignVCenter) {
        // Do nothing
    }
    if (m_textShapeData->resizeMethod() == KoTextShapeData::AutoGrowWidthAndHeight
            || m_textShapeData->resizeMethod() == KoTextShapeData::AutoGrowHeight) {
        qreal height = rootArea->bottom() - rootArea->top();
        if (height > newSize.height()) {
            newSize.setHeight(height);
        }
        if (m_textShape->shapeId() == "AnnotationTextShapeID") {
            if (height < newSize.height()) {
                newSize.setHeight(rootArea->bottom() - rootArea->top());
            }
        }
    }
    if (m_textShapeData->resizeMethod() == KoTextShapeData::AutoGrowWidthAndHeight
            || m_textShapeData->resizeMethod() == KoTextShapeData::AutoGrowWidth) {
        qreal width = rootArea->right() - rootArea->left();
        if (width > newSize.width()) {
            newSize.setWidth(rootArea->right() - rootArea->left());
        }
    }

    qreal newBottom = rootArea->top() + newSize.height();
    KoFlake::Position sizeAnchor = KoFlake::TopLeftCorner;

    if (m_textShapeData->verticalAlignment() & Qt::AlignBottom) {
        if (true /*FIXME test no page based shapes interfering*/) {
            rootArea->setVerticalAlignOffset(newBottom - rootArea->bottom());
            sizeAnchor = KoFlake::BottomLeftCorner;
        }
    }
    if (m_textShapeData->verticalAlignment() & Qt::AlignVCenter) {
        if (true /*FIXME test no page based shapes interfering*/) {
            rootArea->setVerticalAlignOffset((newBottom - rootArea->bottom()) / 2);
            sizeAnchor = KoFlake::CenteredPosition;
        }
    }
    newSize += QSizeF(m_textShapeData->leftPadding() + m_textShapeData->rightPadding(),
                      m_textShapeData->topPadding() + m_textShapeData->bottomPadding());
    if (border) {
        newSize += QSizeF(border->borderWidth(KoBorder::LeftBorder) + border->borderWidth(KoBorder::RightBorder), border->borderWidth(KoBorder::TopBorder) + border->borderWidth(KoBorder::BottomBorder));
    }

    if (newSize != m_textShape->size()) {
        // OO grows to both sides so when to small the initial layouting needs
        // to keep that into account.
        if (m_fixAutogrow) {
            m_fixAutogrow = false;
            QSizeF tmpSize = m_textShape->size();
            tmpSize.setWidth(newSize.width());
            QPointF centerpos = rootArea->associatedShape()->absolutePosition(KoFlake::CenteredPosition);
            m_textShape->setSize(tmpSize);
            m_textShape->setAbsolutePosition(centerpos, KoFlake::CenteredPosition);
            centerpos = rootArea->associatedShape()->absolutePosition(sizeAnchor);
            m_textShape->setSize(newSize);
            m_textShape->setAbsolutePosition(centerpos, sizeAnchor);
        }
        m_textShape->setSize(newSize);
    }

    m_textShape->update(m_textShape->outlineRect());
}

void SimpleRootAreaProvider::updateAll()
{
    if (m_area && m_area->associatedShape()) {
        m_area->associatedShape()->update();
    }
}

QRectF SimpleRootAreaProvider::suggestRect(KoTextLayoutRootArea *rootArea)
{
    //Come up with a rect, but actually we don't need the height, as we set it to infinite below
    // Still better keep it for completeness sake
    QRectF rect(QPointF(), m_textShape->size());
    rect.adjust(m_textShapeData->leftPadding(), m_textShapeData->topPadding(), -m_textShapeData->rightPadding(), - m_textShapeData->bottomPadding());

    KoBorder *border = m_textShape->border();
    if (border) {
        rect.adjust(border->borderWidth(KoBorder::LeftBorder),  border->borderWidth(KoBorder::TopBorder),
                    -border->borderWidth(KoBorder::RightBorder), - border->borderWidth(KoBorder::BottomBorder));
    }

    // In simple cases we always set height way too high so that we have no breaking
    // If the shape grows afterwards or not is handled in doPostLayout()
    rect.setHeight(1E6);

    if (m_textShapeData->resizeMethod() == KoTextShapeData::AutoGrowWidthAndHeight
            || m_textShapeData->resizeMethod() == KoTextShapeData::AutoGrowWidth) {
        rootArea->setNoWrap(1E6);
    }

    // Make sure the size is not negative due to padding and border with
    // This can happen on vertical lines containing text on shape.
    if (rect.width() < 0) {
        rect.setWidth(0);
    }
    return rect;
}

QList<KoTextLayoutObstruction *> SimpleRootAreaProvider::relevantObstructions(KoTextLayoutRootArea *rootArea)
{
    Q_UNUSED(rootArea);

    QList<KoTextLayoutObstruction *> obstructions;
    /*
        m_textShape->boundingRect();
        QList<KoShape *> shapes;
        shapes = manager->shapesAt(canvasRect):
    */
    return obstructions;
}
