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
#include "kis_filter.h"

#include <kdebug.h>
#include <qdom.h>
#include <QString>

#include "kis_filter_registry.h"
#include "kis_transaction.h"
#include "kis_undo_adapter.h"
#include "kis_painter.h"
#include "kis_selection.h"
#include "KoID.h"
#include "kis_canvas_subject.h"
#include "kis_progress_display_interface.h"
#include "kis_types.h"
#include "kis_filter_config_widget.h"


KisFilterConfiguration::KisFilterConfiguration(const KisFilterConfiguration & rhs)
{
    m_name = rhs.m_name;
    m_version = rhs.m_version;
    m_properties = rhs.m_properties;
}

void KisFilterConfiguration::fromXML(const QString & s )
{
    m_properties.clear();

    QDomDocument doc;
    doc.setContent( s );
    QDomElement e = doc.documentElement();
    QDomNode n = e.firstChild();

    m_name = e.attribute("name");
    m_version = e.attribute("version").toInt();

    while (!n.isNull()) {
        // We don't nest elements in filter configuration. For now...
        QDomElement e = n.toElement();
        QString name;
        QString type;
        QString value;

        if (!e.isNull()) {
            if (e.tagName() == "property") {
                name = e.attribute("name");
                type = e.attribute("type");
                value = e.text();
                // XXX Convert the variant pro-actively to the right type?
                m_properties[name] = QVariant(value);
            }
        }
        n = n.nextSibling();
    }
    //dump();
}

QString KisFilterConfiguration::toString()
{
    QDomDocument doc = QDomDocument("filterconfig");
    QDomElement root = doc.createElement( "filterconfig" );
    root.setAttribute( "name", m_name );
    root.setAttribute( "version", m_version );

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

const QString & KisFilterConfiguration::name() const
{
    return m_name;
}

qint32 KisFilterConfiguration::version() const
{
    return m_version;
}

void KisFilterConfiguration::setProperty(const QString & name, const QVariant & value)
{
    if ( m_properties.find( name ) == m_properties.end() ) {
        m_properties.insert( name, value );
    }
    else {
        m_properties[name] = value;
    }
}

bool KisFilterConfiguration::getProperty(const QString & name, QVariant & value)
{
   if ( m_properties.find( name ) == m_properties.end() ) {
       return false;
   }
   else {
       value = m_properties[name];
       return true;
   }
}

QVariant KisFilterConfiguration::getProperty(const QString & name)
{
    if ( m_properties.find( name ) == m_properties.end() ) {
        return QVariant();
    }
    else {
        return m_properties[name];
    }
}


int KisFilterConfiguration::getInt(const QString & name, int def)
{
    QVariant v = getProperty(name);
    if (v.isValid())
        return v.toInt();
    else
        return def;

}

double KisFilterConfiguration::getDouble(const QString & name, double def)
{
    QVariant v = getProperty(name);
    if (v.isValid())
        return v.toDouble();
    else
        return def;
}

bool KisFilterConfiguration::getBool(const QString & name, bool def)
{
    QVariant v = getProperty(name);
    if (v.isValid())
        return v.toBool();
    else
        return def;
}

QString KisFilterConfiguration::getString(const QString & name, QString def)
{
    QVariant v = getProperty(name);
    if (v.isValid())
        return v.toString();
    else
        return def;
}

void KisFilterConfiguration::dump()
{
    QMap<QString, QVariant>::Iterator it;
    for ( it = m_properties.begin(); it != m_properties.end(); ++it ) {
    }

}
