/* This file is part of the KDE project
 * Copyright (C) 2006-2007, 2010 Thomas Zander <zander@kde.org>
 * Copyright (C) 2010 Ko Gmbh <casper.boemann@kogmbh.com>
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
#include "TextLine.h"
#include "Outline.h"

const qreal RIDICULOUSLY_LARGE_NEGATIVE_INDENT = -5E6;
#define MIN_WIDTH   0.01f

TextLine::TextLine()
{
    m_lineRect = QRectF();
    m_updateValidOutlines = false;
    m_horizontalPosition = RIDICULOUSLY_LARGE_NEGATIVE_INDENT;
    m_processingLine = false;
    m_restartOnNextShape = false;
}

void TextLine::createLine(KoTextDocumentLayout::LayoutState *state) {
    m_state = state;
    line = m_state->layout->createLine();
}

void TextLine::setRestartOnNextShape(bool restartOnNextShape)
{
    m_restartOnNextShape = restartOnNextShape;
}

void TextLine::setOutlines(const QList<Outline*> &outlines)
{
    m_outlines = &outlines;
}

bool TextLine::processingLine()
{
    return m_processingLine;
}

void TextLine::updateOutline(Outline *outline)
{
    QRectF outlineLineRect = outline->cropToLine(m_lineRect);
    if (outlineLineRect.isValid()) {
        m_updateValidOutlines = true;
    }
}

void TextLine::fit(const bool resetHorizontalPosition)
{
    if (resetHorizontalPosition) {
        m_horizontalPosition = RIDICULOUSLY_LARGE_NEGATIVE_INDENT;
        m_processingLine = false;
    }
    const qreal maxLineWidth = m_state->width();
    // Make sure at least some text is fitted if the basic width (page, table cell, column)
    // is too small
    if (maxLineWidth <= 0.) {
        // we need to make sure that something like "line.setLineWidth(0.0);" is called here to prevent
        // the QTextLine from being removed again and leading at a later point to crashes. It seems
        // following if-condition including the setNumColumns call was added to do exactly that. But
        // it's not clear for what the if-condition was added. In any case that condition is wrong or
        // incompleted cause things can still crash with m_state->layout->text().length() == 0 (see the
        // document attached to bug 244411).

        //if (m_state->layout->lineCount() > 1 || m_state->layout->text().length() > 0)
            line.setNumColumns(1);

        line.setPosition(QPointF(m_state->x(), m_state->y()));
        return;
    }

    // Too little width because of  wrapping is handled in the remainder of this method
    line.setLineWidth(maxLineWidth);
    const qreal maxLineHeight = line.height();
    const qreal maxNaturalTextWidth = line.naturalTextWidth();
    QRectF lineRect(m_state->x(), m_state->y(), maxLineWidth, maxLineHeight);
    QRectF lineRectPart;
    qreal movedDown = 0;
    if (m_state->maxLineHeight() > 0) {
        movedDown = m_state->maxLineHeight();
    } else {
        movedDown = 10;
    }

    while (!lineRectPart.isValid()) {
        // The line rect could be split into no further linerectpart, so we have
        // to move the lineRect down a bit and try again
        // No line rect part was enough big, to fit the line. Recreate line rect further down
        // (and that is devided into new line parts). Line rect is at different position to
        // outlines, so new parts are completely different. if there are no outlines, then we
        // have only one line part which is full line rect

        lineRectPart = getLineRect(lineRect, maxNaturalTextWidth);
        if (!lineRectPart.isValid()) {
            m_horizontalPosition = RIDICULOUSLY_LARGE_NEGATIVE_INDENT;
            lineRect = QRectF(m_state->x(), m_state->y() + movedDown, maxLineWidth, maxLineHeight);
            movedDown += 10;
        }
    }

    line.setLineWidth(m_textWidth);
    line.setPosition(QPointF(lineRectPart.x(), lineRectPart.y()));
    checkEndOfLine(lineRectPart, maxNaturalTextWidth);
}

void TextLine::validateOutlines()
{
    m_validOutlines.clear();
    foreach (Outline *outline, *m_outlines) {
        validateOutline(outline);
    }
}

void TextLine::validateOutline(Outline *outline)
{
    QRectF outlineLineRect = outline->cropToLine(m_lineRect);
    if (outlineLineRect.isValid()) {
        m_validOutlines.append(outline);
    }
}

void TextLine::createLineParts()
{
    m_lineParts.clear();
    if (m_validOutlines.isEmpty()) {
        // Add whole line rect
        m_lineParts.append(m_lineRect);
    } else {
        QList<QRectF> lineParts;
        QRectF rightLineRect = m_lineRect;
        qSort(m_validOutlines.begin(), m_validOutlines.end(), Outline::compareRectLeft);
        // Devide rect to parts, part can be invalid when outlines are not disjunct.
        foreach (Outline *validOutline, m_validOutlines) {
            QRectF leftLineRect = validOutline->getLeftLinePart(rightLineRect);
            lineParts.append(leftLineRect);
            QRectF lineRect = validOutline->getRightLinePart(rightLineRect);
            if (lineRect.isValid()) {
                rightLineRect = lineRect;
            }
        }
        lineParts.append(rightLineRect);
        Q_ASSERT(m_validOutlines.size() + 1 == lineParts.size());
        // Select invalid parts because of wrap.
        for (int i = 0; i < m_validOutlines.size(); i++) {
            Outline *outline = m_validOutlines.at(i);
            if (outline->noTextAround()) {
                lineParts.replace(i, QRectF());
                lineParts.replace(i + 1, QRect());
            } else if (outline->textOnLeft()) {
                lineParts.replace(i + 1, QRect());
            } else if (outline->textOnRight()) {
                lineParts.replace(i, QRectF());
            } else if (outline->textOnBiggerSide()) {
                QRectF leftReft = outline->getLeftLinePart(m_lineRect);
                QRectF rightRect = outline->getRightLinePart(m_lineRect);
                if (leftReft.width() < rightRect.width()) {
                    lineParts.replace(i, QRectF());
                } else {
                    lineParts.replace(i + 1, QRectF());
                }
            }
        }
        // Filter invalid parts.
        foreach (QRectF rect, lineParts) {
            if (rect.isValid()) {
                m_lineParts.append(rect);
            }
        }
    }
}

QRectF TextLine::minimizeHeightToLeastNeeded(const QRectF &lineRect)
{
    QRectF lineRectBase = lineRect;
    // Get width of one char or shape (as-char).
    m_textWidth = line.cursorToX(line.textStart() + 1) - line.cursorToX(line.textStart());
    // Make sure width is not wider than state allows.
    if (m_textWidth > m_state->width()) {
        m_textWidth = m_state->width();
    }
    line.setLineWidth(m_textWidth);
    // Base linerect height on the width calculated above.
    lineRectBase.setHeight(line.height());
    return lineRectBase;
}

void TextLine::updateLineParts(const QRectF &lineRect)
{
    if (m_lineRect != lineRect || m_updateValidOutlines) {
        m_lineRect = lineRect;
        m_updateValidOutlines = false;
        validateOutlines();
        createLineParts();
    }
}

QRectF TextLine::getLineRectPart()
{
    //TODO korinpa: use binary search tree ?
    QRectF retVal;
    foreach (QRectF lineRectPart, m_lineParts) {
        if (m_horizontalPosition <= lineRectPart.left() && m_textWidth <= lineRectPart.width()) {
            retVal = lineRectPart;
            break;
        }
    }
    return retVal;
}

void TextLine::setMaxTextWidth(const QRectF &minLineRectPart, const qreal leftIndent, const qreal maxNaturalTextWidth) {
    qreal width = m_textWidth;
    qreal maxWidth = minLineRectPart.width() - leftIndent;
    qreal height;
    qreal maxHeight = minLineRectPart.height();
    qreal widthDiff = maxWidth - width;
    //TODO korinpa: use binary shift ?
    widthDiff /= 2;
    while (width <= maxWidth && width <= maxNaturalTextWidth && widthDiff > MIN_WIDTH) {
        line.setLineWidth(width + widthDiff);
        height = line.height();
        if (height <= maxHeight) {
            width = width + widthDiff;
            m_textWidth = width;
        }
        widthDiff /= 2;
    }
}

QRectF TextLine::getLineRect(const QRectF &lineRect, const qreal maxNaturalTextWidth) {
    const qreal leftIndent = lineRect.left();
    QRectF minLineRect = minimizeHeightToLeastNeeded(lineRect);
    updateLineParts(minLineRect);

    // Get appropriate line rect part, to fit line,
    // using horizontal position, minimal height and width of line.
    QRectF lineRectPart = getLineRectPart();
    if (lineRectPart.isValid()) {
        qreal x = lineRectPart.x();
        qreal width = lineRectPart.width();

        // Limit moved the left edge, keep the indent.
        if (leftIndent < x) {
            x += leftIndent;
            width -= leftIndent;
        }
        line.setLineWidth(width);

        // Check if line rect is big enough to fit line.
        // Otherwise find shorter width, what means also shorter height of line.
        // Condition is reverted.
        if (line.height() > lineRectPart.height()) {
            setMaxTextWidth(lineRectPart, leftIndent, maxNaturalTextWidth);
        } else {
            m_textWidth = width;
        }
    }
    return lineRectPart;
}

void TextLine::checkEndOfLine(const QRectF &lineRectPart, const qreal maxNaturalTextWidth) {
    if (lineRectPart == m_lineParts.last() || maxNaturalTextWidth <= lineRectPart.width()) {
        m_horizontalPosition = RIDICULOUSLY_LARGE_NEGATIVE_INDENT;
        m_processingLine = false;
    } else {
        m_horizontalPosition = lineRectPart.right();
        m_processingLine = true;
    }
}
