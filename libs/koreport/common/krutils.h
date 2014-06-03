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

class QDomDocument;
class QDomElement;
class QFont;
class QPointF;
class QSizeF;
class QString;
class KRPos;
class KRSize;
class KRTextStyleData;
class KRLineStyleData;

namespace KoProperty {
    class Property;
}

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

    //! Writes attributes for the rect position @p pos, @p siz
    void buildXMLRect(QDomElement &entity, KRPos *pos, KRSize *siz);
    //! Writes attributes for text style @p ts
    void buildXMLTextStyle(QDomDocument & doc, QDomElement & entity, KRTextStyleData ts);
    //! Writes attributes for line style @p ls
    void buildXMLLineStyle(QDomDocument & doc, QDomElement & entity, KRLineStyleData ls);
    //! Writes attributes for the property @p p
    void addPropertyAsAttribute(QDomElement* e, KoProperty::Property* p);
    //! Writes @p attribute to element @p e, @p value is stored in points with unit 'pt'
    void setAttribute(QDomElement &e, const QString &attribute, double value);
    //! Writes point @p value as attributes to element @p e
    void setAttribute(QDomElement &e, const QPointF &value);
    //! Writes size @p value as attributes to element @p e
    void setAttribute(QDomElement &e, const QSizeF &value);
    //! Reads attributes from @p elemSource into text style @p ts
    bool parseReportTextStyleData(const QDomElement & elemSource, KRTextStyleData & ts);
    //! Reads attributes from @p elemSource into line style @p ls
    bool parseReportLineStyleData(const QDomElement & elemSource, KRLineStyleData & ls);
    //! Reads attributes from @p elemSource into rect @p pos, @p siz
    bool parseReportRect(const QDomElement & elemSource, KRPos *pos, KRSize *siz);
}

#endif

