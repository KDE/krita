/* This file is part of the KDE project
 * Copyright (C) 2006-2010 Thomas Zander <zander@kde.org>
 * Copyright (C) 2008,2010 Thorsten Zachmann <zachmann@kde.org>
 * Copyright (C) 2008 Girish Ramakrishnan <girish@forwardbias.in>
 * Copyright (C) 2009 KO GmbH <cbo@kogmbh.com>
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
#ifndef KOTABLEBORDERSTYLE_H
#define KOTABLEBORDERSTYLE_H

#include "KoText.h"
#include "kotext_export.h"
#include <KoBorder.h>

class KoTableBorderStylePrivate;

class KOTEXT_EXPORT KoTableBorderStyle
{
public:
    enum Property {
        StyleId = QTextTableCellFormat::UserProperty + 7001,
        TopBorderOuterPen, ///< the top border pen
        TopBorderSpacing,          ///< the top border spacing between inner and outer border
        TopBorderInnerPen,       ///< the top border inner pen
        TopBorderStyle,       ///< the top border borderstyle
        LeftBorderOuterPen,      ///< the left border outer pen
        LeftBorderSpacing,         ///< the left border spacing between inner and outer border
        LeftBorderInnerPen,      ///< the left border inner pen
        LeftBorderStyle,       ///< the left border borderstyle
        BottomBorderOuterPen,    ///< the bottom border outer pen
        BottomBorderSpacing,       ///< the bottom border spacing between inner and outer border
        BottomBorderInnerPen,    ///< the bottom border inner pen
        BottomBorderStyle,       ///< the bottom border borderstyle
        RightBorderOuterPen,     ///< the right border outer pen
        RightBorderSpacing,        ///< the right border spacing between inner and outer border
        RightBorderInnerPen,     ///< the right border inner pen
        RightBorderStyle,       ///< the right border borderstyle
        TopLeftToBottomRightBorderOuterPen, ///< the top left to bottom right diagonal border pen
        TopLeftToBottomRightBorderSpacing,  ///< the top left to bottom right diagonal spacing
        TopLeftToBottomRightBorderInnerPen, ///< the top left to bottom right diagonal inner pen
        TopLeftToBottomRightBorderStyle,    ///< the top left to bottom right borderstyle
        BottomLeftToTopRightBorderOuterPen, ///< the bottom letf to top right diagonal border pen
        BottomLeftToTopRightBorderSpacing,  ///< the bottom letf to top right diagonal spacing
        BottomLeftToTopRightBorderInnerPen, ///< the bottom letf to top right diagonal inner pen
        BottomLeftToTopRightBorderStyle,    ///< the bottom letf to top right borderstyle
    };

    enum Side {
        Top = 0, ///< References the border at the top of the cell
        Left,    ///< References the border at the left side of the cell
        Bottom,  ///< References the border at the bottom of the cell
        Right,   ///< References the border at the right side of the paragraph
        TopLeftToBottomRight, ///< References the border from top, left corner to bottom, right corner of cell
        BottomLeftToTopRight  ///< References the border from bottom, left corner to top, right corner of cell
    };

    struct Edge {
        Edge() : innerPen(), outerPen(), spacing(0.0) { }

        QPen innerPen;
        QPen outerPen;
        qreal spacing;
    };
};

#endif // KOTABLEBORDERSTYLE_H

