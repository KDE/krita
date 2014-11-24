/* This file is part of the KDE project
 * Copyright (C) 2006-2007, 2010 Thomas Zander <zander@kde.org>
 * Copyright (C) 2010-2011 KO Gmbh <cbo@kogmbh.com>
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
#include "RunAroundHelper.h"

#include "KoTextLayoutObstruction.h"

#include "KoTextLayoutArea.h"

const qreal RIDICULOUSLY_LARGE_NEGATIVE_INDENT = -5E6;
#define MIN_WIDTH   0.01f

RunAroundHelper::RunAroundHelper()
{
    m_lineRect = QRectF();
    m_updateValidObstructions = false;
    m_horizontalPosition = RIDICULOUSLY_LARGE_NEGATIVE_INDENT;
    m_stayOnBaseline = false;
}

void RunAroundHelper::setLine(KoTextLayoutArea *area, const QTextLine &l) {
    m_area = area;
    line = l;
}

void RunAroundHelper::setObstructions(const QList<KoTextLayoutObstruction*> &obstructions)
{
    m_obstructions = obstructions;
}

bool RunAroundHelper::stayOnBaseline() const
{
    return m_stayOnBaseline;
}

void RunAroundHelper::updateObstruction(KoTextLayoutObstruction *obstruction)
{
    QRectF obstructionLineRect = obstruction->cropToLine(m_lineRect);
    if (obstructionLineRect.isValid()) {
        m_updateValidObstructions = true;
    }
}

bool RunAroundHelper::fit(const bool resetHorizontalPosition, bool isRightToLeft, const QPointF &position)
{
    Q_ASSERT(line.isValid());
    if (resetHorizontalPosition) {
        m_horizontalPosition = RIDICULOUSLY_LARGE_NEGATIVE_INDENT;
        m_stayOnBaseline = false;
    }
    const qreal maxLineWidth = m_area->width();
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

        line.setPosition(position);
        return false;
    }

    // Too little width because of  wrapping is handled in the remainder of this method
    line.setLineWidth(maxLineWidth);
    const qreal maxLineHeight = line.height();
    const qreal maxNaturalTextWidth = line.naturalTextWidth();
    QRectF lineRect(position, QSizeF(maxLineWidth, maxLineHeight));
    QRectF lineRectPart;
    qreal movedDown = 10;

    while (!lineRectPart.isValid()) {
        // The line rect could be split into no further linerectpart, so we have
        // to move the lineRect down a bit and try again
        // No line rect part was enough big, to fit the line. Recreate line rect further down
        // (and that is divided into new line parts). Line rect is at different position to
        // obstructions, so new parts are completely different. if there are no obstructions, then we
        // have only one line part which is full line rect

        lineRectPart = getLineRect(lineRect, maxNaturalTextWidth);
        if (!lineRectPart.isValid()) {
            m_horizontalPosition = RIDICULOUSLY_LARGE_NEGATIVE_INDENT;
            lineRect = QRectF(position, QSizeF(maxLineWidth, maxLineHeight));
            lineRect.setY(lineRect.y() + movedDown);
            movedDown += 10;
        }
    }

    if (isRightToLeft && line.naturalTextWidth() > m_textWidth) {
        // This can happen if spaces are added at the end of a line. Those spaces will not result in a
        // line-break. On left-to-right everything is fine and the spaces at the end are just not visible
        // but on right-to-left we need to adust the position cause spaces at the end are displayed at
        // the beginning and we need to make sure that doesn't result in us cutting of text at the right side.
        qreal diff = line.naturalTextWidth() - m_textWidth;
        lineRectPart.setX(lineRectPart.x() - diff);
    }

    line.setLineWidth(m_textWidth);
    line.setPosition(QPointF(lineRectPart.x(), lineRectPart.y()));
    checkEndOfLine(lineRectPart, maxNaturalTextWidth);
    return true;
}

void RunAroundHelper::validateObstructions()
{
    m_validObstructions.clear();
    foreach (KoTextLayoutObstruction *obstruction, m_obstructions) {
        validateObstruction(obstruction);
    }
}

void RunAroundHelper::validateObstruction(KoTextLayoutObstruction *obstruction)
{
    QRectF obstructionLineRect = obstruction->cropToLine(m_lineRect);
    if (obstructionLineRect.isValid()) {
        m_validObstructions.append(obstruction);
    }
}

void RunAroundHelper::createLineParts()
{
    m_lineParts.clear();
    if (m_validObstructions.isEmpty()) {
        // Add whole line rect
        m_lineParts.append(m_lineRect);
    } else {
        QList<QRectF> lineParts;
        QRectF rightLineRect = m_lineRect;
        bool lastRightRectValid = false;
        qSort(m_validObstructions.begin(), m_validObstructions.end(), KoTextLayoutObstruction::compareRectLeft);
        // Divide rect to parts, part can be invalid when obstructions are not disjunct.
        foreach (KoTextLayoutObstruction *validObstruction, m_validObstructions) {
            QRectF leftLineRect = validObstruction->getLeftLinePart(rightLineRect);
            lineParts.append(leftLineRect);
            QRectF lineRect = validObstruction->getRightLinePart(rightLineRect);
            if (lineRect.isValid()) {
                rightLineRect = lineRect;
                lastRightRectValid = true;
            } else {
                lastRightRectValid = false;
            }
        }
        if (lastRightRectValid) {
            lineParts.append(rightLineRect);
        }
        else {
            lineParts.append(QRect());
        }
        Q_ASSERT(m_validObstructions.size() + 1 == lineParts.size());
        // Select invalid parts because of wrap.
        for (int i = 0; i < m_validObstructions.size(); i++) {
            KoTextLayoutObstruction *obstruction = m_validObstructions.at(i);
            if (obstruction->noTextAround()) {
                lineParts.replace(i, QRectF());
                lineParts.replace(i + 1, QRect());
            } else if (obstruction->textOnLeft()) {
                lineParts.replace(i + 1, QRect());
            } else if (obstruction->textOnRight()) {
                lineParts.replace(i, QRectF());
            } else if (obstruction->textOnEnoughSides()) {
                QRectF leftRect = obstruction->getLeftLinePart(m_lineRect);
                QRectF rightRect = obstruction->getRightLinePart(m_lineRect);
                if (leftRect.width() < obstruction->runAroundThreshold()) {
                    lineParts.replace(i, QRectF());
                }
                if (rightRect.width() < obstruction->runAroundThreshold()) {
                    lineParts.replace(i + 1, QRectF());
                }
            } else if (obstruction->textOnBiggerSide()) {
                QRectF leftRect = obstruction->getLeftLinePart(m_lineRect);
                QRectF rightRect = obstruction->getRightLinePart(m_lineRect);
                if (leftRect.width() < rightRect.width()) {
                    lineParts.replace(i, QRectF());
                } else {
                    lineParts.replace(i + 1, QRectF());
                }
            }
        }
        // Filter invalid parts.
        foreach (const QRectF &rect, lineParts) {
            if (rect.isValid()) {
                m_lineParts.append(rect);
            }
        }
    }
}

QRectF RunAroundHelper::minimizeHeightToLeastNeeded(const QRectF &lineRect)
{
    Q_ASSERT(line.isValid());
    QRectF lineRectBase = lineRect;
    // Get width of one char or shape (as-char).
    m_textWidth = line.cursorToX(line.textStart() + 1) - line.cursorToX(line.textStart());
    // Make sure width is not wider than the area allows.
    if (m_textWidth > m_area->width()) {
        m_textWidth = m_area->width();
    }
    line.setLineWidth(m_textWidth);
    // Base linerect height on the width calculated above.
    lineRectBase.setHeight(line.height());
    return lineRectBase;
}

void RunAroundHelper::updateLineParts(const QRectF &lineRect)
{
    if (m_lineRect != lineRect || m_updateValidObstructions) {
        m_lineRect = lineRect;
        m_updateValidObstructions = false;
        validateObstructions();
        createLineParts();
    }
}

QRectF RunAroundHelper::getLineRectPart()
{
    QRectF retVal;
    foreach (const QRectF &lineRectPart, m_lineParts) {
        if (m_horizontalPosition <= lineRectPart.left() && m_textWidth <= lineRectPart.width()) {
            retVal = lineRectPart;
            break;
        }
    }
    return retVal;
}

void RunAroundHelper::setMaxTextWidth(const QRectF &minLineRectPart, const qreal leftIndent, const qreal maxNaturalTextWidth)
{
    Q_ASSERT(line.isValid());
    qreal width = m_textWidth;
    qreal maxWidth = minLineRectPart.width() - leftIndent;
    qreal height;
    qreal maxHeight = minLineRectPart.height();
    qreal widthDiff = maxWidth - width;

    widthDiff /= 2;
    while (width <= maxWidth && width <= maxNaturalTextWidth && widthDiff > MIN_WIDTH) {
        qreal linewidth = width + widthDiff;
        line.setLineWidth(linewidth);
        height = line.height();
        if (height <= maxHeight) {
            width = linewidth;
            m_textWidth = width;
        }
        widthDiff /= 2;
    }
}

QRectF RunAroundHelper::getLineRect(const QRectF &lineRect, const qreal maxNaturalTextWidth)
{
    Q_ASSERT(line.isValid());

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

void RunAroundHelper::checkEndOfLine(const QRectF &lineRectPart, const qreal maxNaturalTextWidth)
{
    if (lineRectPart == m_lineParts.last() || maxNaturalTextWidth <= lineRectPart.width()) {
        m_horizontalPosition = RIDICULOUSLY_LARGE_NEGATIVE_INDENT;
        m_stayOnBaseline = false;
    } else {
        m_horizontalPosition = lineRectPart.right();
        m_stayOnBaseline = true;
    }
}
