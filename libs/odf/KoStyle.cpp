/*
 *  Copyright (c) 2010 Carlos Licea <carlos@kdab.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published
 *  by the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "KoStyle.h"
#include "KoGenStyles.h"

KoStyle::KoStyle()
: m_autoStyle(false), m_autoStyleInStylesDotXml(false)
{
}

KoStyle::~KoStyle()
{
}

void KoStyle::setName(QString name)
{
    m_name = name;
}

QString KoStyle::name() const
{
    return m_name;
}

void KoStyle::setAutoStyleInStylesDotXml(bool b)
{
    m_autoStyleInStylesDotXml = b;
}

bool KoStyle::autoStyleInStylesDotXml() const
{
    return m_autoStyleInStylesDotXml;
}

KoGenStyles::InsertionFlags KoStyle::insertionFlags() const
{
    if(m_name.isEmpty()) {
        return KoGenStyles::NoFlag;
    }
    else {
        return KoGenStyles::DontAddNumberToName | KoGenStyles::AllowDuplicates;
    }
}

QString KoStyle::saveOdf(KoGenStyles& styles) const
{
    KoGenStyle::Type type;
    if(m_name.isEmpty()) {
        type = automaticstyleType();
    }
    else {
        type = styleType();
    }
    KoGenStyle style(type, styleFamilyName());
    prepareStyle(style);
    style.setAutoStyleInStylesDotXml(m_autoStyleInStylesDotXml);

    QString styleName = m_name;
    if(styleName.isEmpty()) {
        styleName = defaultPrefix();
    }

    return styles.insert(style, styleName, insertionFlags());
}
