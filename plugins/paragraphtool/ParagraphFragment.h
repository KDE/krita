/* This file is part of the KDE project
 * Copyright (C) 2008 Florian Merz <florianmerz@gmx.de>
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

#ifndef PARAGRAPHFRAGMENT_H
#define PARAGRAPHFRAGMENT_H

#include <QRectF>

class KoParagraphStyle;
class KoShape;
class QTextBlock;

/* ParagraphFragment is used by ParagraphTool to store information about a
 * paragraph which is specific to a shape. As the width of shapes may be
 * different the width of a single paragraph on different shapes has to be
 * different, too.
 */
class ParagraphFragment
{
public:
    ParagraphFragment() {};
    ParagraphFragment(KoShape *shape, const QTextBlock &textBlock, KoParagraphStyle *style);

    ~ParagraphFragment() {};

    KoShape *shape() const { return m_shape; }

    QRectF listCounter() const { return m_counter; }
    QRectF firstLine() const { return m_firstLine; }
    QRectF followingLines() const { return m_followingLines; }
    QRectF border() const { return m_border; }

    bool isSingleLine() const { return m_isSingleLine; }

private:
    KoShape *m_shape;

    QRectF m_counter,
    m_firstLine,
    m_followingLines,
    m_border;

    bool m_isSingleLine;
};

#endif

