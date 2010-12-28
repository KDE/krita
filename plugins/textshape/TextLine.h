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

#include "Outline.h"
#include <KoTextDocumentLayout.h>

#include <QList>
#include <QTextLine>
#include <QRectF>

class Line
{
public:
    Line();
    void createLine(KoTextDocumentLayout::LayoutState *state);
    void setRestartOnNextShape(bool restartOnNextShape);
    bool isValid() const;
    void setOutlines(const QList<Outline*> &outlines);
    bool processingLine();
    void updateOutline(Outline *outline);
    void fit(const bool resetHorizontalPosition = false);
    QTextLine line;
private:
    KoTextDocumentLayout::LayoutState *m_state;
    const QList<Outline*> *m_outlines;
    QList<Outline*> m_validOutlines;
    QList<QRectF> m_lineParts;
    QRectF m_lineRect;
    qreal m_horizontalPosition;
    bool m_updateValidOutlines;
    bool m_processingLine;
    qreal m_textWidth;
    bool m_restartOnNextShape;
    void validateOutlines();
    void validateOutline(Outline *outline);
    void createLineParts();
    QRectF minimizeHeightToLeastNeeded(const QRectF &lineRect);
    void updateLineParts(const QRectF &lineRect);
    QRectF getLineRectPart();
    void setMaxTextWidth(const QRectF &minLineRectPart, const qreal leftIndent, const qreal maxNaturalTextWidth);
    QRectF getLineRect(const QRectF &lineRect, const qreal maxNaturalTextWidth);
    void checkEndOfLine(const QRectF &lineRectPart, const qreal maxNaturalTextWidth);
};
