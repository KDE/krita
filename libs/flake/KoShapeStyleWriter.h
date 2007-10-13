/* This file is part of the KDE project
   Copyright (C) 2007 Jan Hambrecht <jaham@gmx.net>

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

#ifndef KOSHAPESTYLEWRITER_H
#define KOSHAPESTYLEWRITER_H

#include <flake_export.h>
#include <QtCore/QString>

class KoShapeSavingContext;
class KoGenStyle;
class KoXmlWriter;
class QBrush;

/// A helper class for writing the shape styles to ODF.
class FLAKE_EXPORT KoShapeStyleWriter
{
public:
    /// Creates a new shape style writer working on the given saving context
    KoShapeStyleWriter( KoShapeSavingContext &context );

    /**
     * Writes the style of the brush using the internal saving context
     * @param style the style to write to
     * @param brush the fill style to save
     * @return the name of the saved style
     */
    QString addFillStyle( KoGenStyle &style, const QBrush &brush );

private:
    /// Saves pattern style
    QString savePatternStyle( KoGenStyle &style, const QBrush &brush );

    KoShapeSavingContext & m_context; ///< the shape saving context the styles are added to
};

#endif // KOSHAPESTYLEWRITER_H
