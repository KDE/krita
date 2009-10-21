/*
 * Copyright (c) 2009 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "Extension.h"

#include <QDomElement>

Extension::Extension()
{
}

Extension::~Extension()
{
}

const QString& Extension::name() const
{
    return m_name;
}

const QString& Extension::description() const
{
    return m_description;
}

const QString& Extension::version() const
{
    return m_version;
}

#include <kdebug.h>

QString getChildText(const QDomElement& element)
{
    QDomText text = element.firstChild().toText();
    return text.data();
}

void Extension::parse(const QDomElement& element)
{
    Q_ASSERT(m_name.isEmpty());
    Q_ASSERT(m_description.isEmpty());
    Q_ASSERT(m_version.isEmpty());
    QDomNode n = element.firstChild();
    while (!n.isNull()) {
        if (n.isElement()) {
            QDomElement e = n.toElement();
            if (e.tagName() == "name") {
                m_name = getChildText(e);
            } else if (e.tagName() == "description") {
                m_description = getChildText(e);
            } else if (e.tagName() == "version") {
                m_version = getChildText(e);
            }
        }
        n = n.nextSibling();
    }

}
