/* This file is part of the KDE project
   Copyright (C) 2004-2006 David Faure <faure@kde.org>
   Copyright (C) 2007 Jan Hambrecht <jaham@gmx.net>
   Copyright (C) 2007 Thorsten Zachmann <zachmann@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

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

#include "koodf_export.h"

#include <QtGui/QMatrix>

class QBrush;
class QPen;
class QString;
class QSizeF;

class KoGenStyle;
class KoGenStyles;
class KoStyleStack;

class KoOdfStylesReader;
class KoOdfLoadingContext;

namespace KoOdfGraphicStyles
{
    KOODF_EXPORT void saveOdfFillStyle(KoGenStyle &styleFill, KoGenStyles& mainStyles, const QBrush &brush);

    KOODF_EXPORT void saveOdfStrokeStyle(KoGenStyle &styleStroke, KoGenStyles &mainStyles, const QPen &pen);

    KOODF_EXPORT QString saveOdfHatchStyle(KoGenStyles &mainStyles, const QBrush &brush);

    /// Saves gradient style of brush into mainStyles and returns the styles name
    KOODF_EXPORT QString saveOdfGradientStyle(KoGenStyles &mainStyles, const QBrush &brush);

    /// Loads gradient style from style stack and stylesReader adapted to the given size and returns a brush
    KOODF_EXPORT QBrush loadOdfGradientStyle(const KoStyleStack &styleStack, const KoOdfStylesReader &stylesReader, const QSizeF &size);

    /// Loads gradient style with the given name from style stack and stylesReader adapted to the given size and returns a brush
    KOODF_EXPORT QBrush loadOdfGradientStyleByName(const KoOdfStylesReader &stylesReader, const QString &styleName, const QSizeF &size);

    KOODF_EXPORT QBrush loadOdfFillStyle(const KoStyleStack &styleStack, const QString &fill,  const KoOdfStylesReader &stylesReader);

    KOODF_EXPORT QPen loadOdfStrokeStyle(const KoStyleStack &styleStack, const QString &stroke, const KoOdfStylesReader &stylesReader);

    /// Helper function to parse a transformation attribute
    KOODF_EXPORT QMatrix loadTransformation(const QString &transformation);

    /// Helper function to create a transformation attribute
    KOODF_EXPORT QString saveTransformation(const QMatrix &transformation, bool appendTranslateUnit = true);
};

#endif /* KOODFGRAPHICSTYLES_H */
