/* This file is part of the KDE project
   Copyright (C) 2010 Jarosław Staniek <staniek@kde.org>

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

#ifndef KRUTILS_H
#define KRUTILS_H

class QDomElement;
class QFont;

namespace KRUtils
{
    //! @return percent value for element @a name. If the element is missing, returns @a defaultPercentValue.
    //! If @a ok is not 0, *ok is set to the result.
    int readPercent(const QDomElement & el, const char* name, int defaultPercentValue, bool *ok);

    //! Reads all font attributes for element @a el into @a font.
    //! @todo add unit tests
    bool readFontAttributes(const QDomElement& el, QFont& font);

    //! Writes all attributes of font @a font into element @a el.
    //! @todo add unit tests
    void writeFontAttributes(QDomElement& el, const QFont &font);
}

#endif

