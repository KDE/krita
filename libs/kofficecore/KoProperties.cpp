/*
 *  Copyright (c) 2006 Boudewijn Rempt <boud@valdyas.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#include <kdebug.h>
#include <qdom.h>
#include <QString>

#include "KoProperties.h"

KoProperties::KoProperties(const KoProperties & rhs)
    : m_properties( rhs.m_properties )
{
}

void KoProperties::load(const QString & s)
{
    m_properties.clear();

    QDomDocument doc;
    doc.setContent( s );
    QDomElement e = doc.documentElement();
    QDomNode n = e.firstChild();

    while (!n.isNull()) {
        // We don't nest elements in filter configuration. For now...
        QDomElement e = n.toElement();
        if (!e.isNull()) {
            if (e.tagName() == "property") {
                const QString name = e.attribute("name");
                const QString type = e.attribute("type");
                const QString value = e.text();
                // XXX Convert the variant pro-actively to the right type?
                m_properties[name] = QVariant(value);
            }
        }
        n = n.nextSibling();
    }
}

QString KoProperties::store()
{
    QDomDocument doc = QDomDocument("filterconfig");
    QDomElement root = doc.createElement( "filterconfig" );

    doc.appendChild( root );

    QMap<QString, QVariant>::Iterator it;
    for ( it = m_properties.begin(); it != m_properties.end(); ++it ) {
        QDomElement e = doc.createElement( "property" );
        e.setAttribute( "name", QString(it.key().toLatin1()) );
        QVariant v = it.value();
        e.setAttribute( "type", v.typeName() );
        QString s = v.toString();
        QDomText text = doc.createCDATASection(v.toString() ); // XXX: Unittest this!
        e.appendChild(text);
        root.appendChild(e);
    }

    return doc.toString();
}


void KoProperties::setProperty(const QString & name, const QVariant & value)
{
    // If there's an existing value for this name already, replace it.
    m_properties.insert( name, value );
}

bool KoProperties::getProperty(const QString & name, QVariant & value)
{
   QMap<QString, QVariant>::const_iterator it = m_properties.find( name ); 
   if ( it == m_properties.end() ) {
       return false;
   }
   else {
       value = *it;
       return true;
   }
}

QVariant KoProperties::getProperty(const QString & name)
{
    return m_properties.value( name, QVariant() );
}


int KoProperties::getInt(const QString & name, int def)
{
    const QVariant v = getProperty(name);
    if (v.isValid())
        return v.toInt();
    else
        return def;

}

double KoProperties::getDouble(const QString & name, double def)
{
    const QVariant v = getProperty(name);
    if (v.isValid())
        return v.toDouble();
    else
        return def;
}

bool KoProperties::getBool(const QString & name, bool def)
{
    const QVariant v = getProperty(name);
    if (v.isValid())
        return v.toBool();
    else
        return def;
}

QString KoProperties::getString(const QString & name, QString def)
{
    const QVariant v = getProperty(name);
    if (v.isValid())
        return v.toString();
    else
        return def;
}

void KoProperties::dump()
{
    QMap<QString, QVariant>::Iterator it;
    for ( it = m_properties.begin(); it != m_properties.end(); ++it ) {
        // ### TODO finish
    }
}
