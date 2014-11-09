/* This file is part of the KDE project
 * Copyright (C) 2006-2007, 2010 Thomas Zander <zander@kde.org>
 * Copyright (C) 2010 Ko Gmbh <cbo@kogmbh.com>
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
#ifndef RUNAROUNDHELPER_H
#define RUNAROUNDHELPER_H

#include <QList>
#include <QTextLine>
#include <QRectF>

class KoTextLayoutArea;
class KoTextLayoutObstruction;

class RunAroundHelper
{
public:
    RunAroundHelper();
    void setLine(KoTextLayoutArea *area, const QTextLine &l);
    void setObstructions(const QList<KoTextLayoutObstruction *> &obstructions);
    bool stayOnBaseline() const;
    void updateObstruction(KoTextLayoutObstruction *obstruction);
    bool fit(bool resetHorizontalPosition, bool isRightToLeft, const QPointF &position);
    QTextLine line;
private:
    KoTextLayoutArea *m_area;
    QList<KoTextLayoutObstruction*> m_obstructions;
    QList<KoTextLayoutObstruction*> m_validObstructions;
    QList<QRectF> m_lineParts;
    QRectF m_lineRect;
    qreal m_horizontalPosition;
    bool m_updateValidObstructions;
    bool m_stayOnBaseline;
    qreal m_textWidth;
    void validateObstructions();
    void validateObstruction(KoTextLayoutObstruction *obstruction);
    void createLineParts();
    QRectF minimizeHeightToLeastNeeded(const QRectF &lineRect);
    void updateLineParts(const QRectF &lineRect);
    QRectF getLineRectPart();
    void setMaxTextWidth(const QRectF &minLineRectPart, const qreal leftIndent, const qreal maxNaturalTextWidth);
    QRectF getLineRect(const QRectF &lineRect, const qreal maxNaturalTextWidth);
    void checkEndOfLine(const QRectF &lineRectPart, const qreal maxNaturalTextWidth);
};

#endif
