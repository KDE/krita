/* This file is part of the KDE project
   Copyright (C) 2004-2006 David Faure <faure@kde.org>
   Copyright (C) 2007 Jan Hambrecht <jaham@gmx.net>
   Copyright (C) 2007 Thorsten Zachmann <zachmann@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#ifndef KOODFGRAPHICSTYLES_H
#define KOODFGRAPHICSTYLES_H

#include <komain_export.h>

class QBrush;
class QPen;
class QString;
class QSizeF;

class KoGenStyle;
class KoGenStyles;
class KoStyleStack;

class KoOasisStyles;
class KoOasisLoadingContext;

class KOMAIN_EXPORT KoOdfGraphicStyles
{
public:
    static void saveOasisFillStyle( KoGenStyle &styleFill, KoGenStyles& mainStyles, const QBrush & brush );

    static QString saveOasisHatchStyle( KoGenStyles& mainStyles, const QBrush &brush );

    /// Saves gradient style of brush into mainStyles and returns the styles name
    static QString saveOasisGradientStyle( KoGenStyles &mainStyles, const QBrush &brush );

    /// Loads gradient style from style stack and oasisStyles adapted to the given size and returns a brush
    static QBrush loadOasisGradientStyle( const KoStyleStack &styleStack, const KoOasisStyles & oasisStyles, const QSizeF &size );

    /// Loads pattern style from style stack and oasisstyle adapted to the given size
    static QBrush loadOasisPatternStyle( const KoStyleStack &styleStack, KoOasisLoadingContext & context, const QSizeF &size );

    static QBrush loadOasisFillStyle( const KoStyleStack &styleStack, const QString & fill,  const KoOasisStyles & oasisStyles );

    static QPen loadOasisStrokeStyle( const KoStyleStack &styleStack, const QString & stroke, const KoOasisStyles & oasisStyles );
};

#endif /* KOODFGRAPHICSTYLES_H */
