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

#include <qdom.h>
#include <qstring.h>

#include "kis_filter_registry.h"
#include "kis_transaction.h"
#include "kis_undo_adapter.h"
#include "kis_painter.h"
#include "kis_selection.h"
#include "kis_id.h"
#include "kis_canvas_subject.h"
#include "kis_progress_display_interface.h"
#include "kis_types.h"
#include "kis_filter_config_widget.h"


KisFilterConfiguration::KisFilterConfiguration(const QString & s )
{
    QDomDocument doc;
    doc.setContent( s );
    QDomElement e = doc.documentElement();
    QDomNode n = e.firstChild();

    while (!n.isNull()) {
        // We don't nest elements in filter configuration. For now...
        QDomElement e = n.toElement();
        if (!e.isNull()) {
            // Do stuff
        }
        n = n.nextSibling();
    }
    init();
}

KisFilterConfiguration::KisFilterConfiguration(const KisFilterConfiguration & rhs)
{
    m_name = rhs.m_name;
    m_version = rhs.m_version;
    m_properties = rhs.m_properties;
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
        e.setAttribute( "name", it.key().latin1() );
        QVariant v = it.data();
        e.setAttribute( "type", v.typeName() );
        e.setAttribute( "value", v.asString() ); // XXX: Unittest this!
    }

    return doc.toString();
}

const QString & KisFilterConfiguration::name() const
{
    return m_name;
}

Q_INT32 KisFilterConfiguration::version() const
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

