/* This file is part of the KDE project
   Copyright (C) 2004 Laurent Montel <montel@kde.org>

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

#include "KoOasisSettings.h"
#include "KoXmlNS.h"
#include "KoDom.h"
#include <kdebug.h>

KoOasisSettings::KoOasisSettings( const KoXmlDocument& doc )
    : m_settingsElement( KoDom::namedItemNS( doc.documentElement(), KoXmlNS::office, "settings" ) ),
      m_configNSURI( KoXmlNS::config )
{
    const KoXmlElement contents = doc.documentElement();
    if ( m_settingsElement.isNull() )
        kDebug() << " document doesn't have tag 'office:settings'\n";
}

KoOasisSettings::KoOasisSettings( const KoXmlDocument& doc, const char* officeNSURI, const char* configNSURI )
    : m_settingsElement( KoDom::namedItemNS( doc.documentElement(), officeNSURI, "settings" ) ),
      m_configNSURI( configNSURI )
{
    const KoXmlElement contents = doc.documentElement();
    if ( m_settingsElement.isNull() )
        kDebug() << " document doesn't have tag 'office:settings'\n";
}

KoOasisSettings::Items KoOasisSettings::itemSet( const QString& itemSetName ) const
{
    KoXmlElement e;
    forEachElement( e, m_settingsElement )
    {
        if ( e.localName() == "config-item-set" &&
             e.namespaceURI() == m_configNSURI &&
             e.attributeNS( m_configNSURI, "name", QString::null ) == itemSetName )
        {
            return Items( e, this );
        }
    }

    return Items( KoXmlElement(), this );
}

KoOasisSettings::IndexedMap KoOasisSettings::Items::indexedMap( const QString& itemMapName ) const
{
    KoXmlElement configItem;
    forEachElement( configItem, m_element )
    {
        if ( configItem.localName() == "config-item-map-indexed" &&
             configItem.namespaceURI() == m_settings->m_configNSURI &&
             configItem.attributeNS( m_settings->m_configNSURI, "name", QString::null ) == itemMapName )
        {
            return IndexedMap( configItem, m_settings );
        }
    }
    return IndexedMap( KoXmlElement(), m_settings );
}

KoOasisSettings::NamedMap KoOasisSettings::Items::namedMap( const QString& itemMapName ) const
{
    KoXmlElement configItem;
    forEachElement( configItem, m_element )
    {
        if ( configItem.localName() == "config-item-map-named" &&
             configItem.namespaceURI() == m_settings->m_configNSURI &&
             configItem.attributeNS( m_settings->m_configNSURI, "name", QString::null ) == itemMapName )
        {
            return NamedMap( configItem, m_settings );
        }
    }
    return NamedMap( KoXmlElement(), m_settings );
}

KoOasisSettings::Items KoOasisSettings::IndexedMap::entry( int entryIndex ) const
{
    int i = 0;
    KoXmlElement entry;
    forEachElement( entry, m_element )
    {
        if ( entry.localName() == "config-item-map-entry" &&
             entry.namespaceURI() == m_settings->m_configNSURI )
        {
            if ( i == entryIndex )
                return Items( entry, m_settings );
            else
                ++i;
        }
    }
    return Items( KoXmlElement(), m_settings );
}

KoOasisSettings::Items KoOasisSettings::NamedMap::entry( const QString& entryName ) const
{
    KoXmlElement entry;
    forEachElement( entry, m_element )
    {
        if ( entry.localName() == "config-item-map-entry" &&
             entry.namespaceURI() == m_settings->m_configNSURI &&
             entry.attributeNS( m_settings->m_configNSURI, "name", QString::null ) == entryName )
        {
            return Items( entry, m_settings );
        }
    }
    return Items( KoXmlElement(), m_settings );
}

// helper method
QString KoOasisSettings::Items::findConfigItem( const KoXmlElement& element,
                                                const QString& item, bool* ok ) const
{
    KoXmlElement it;
    forEachElement( it, element )
    {
        if ( it.localName() == "config-item" &&
             it.namespaceURI() == m_settings->m_configNSURI &&
             it.attributeNS( m_settings->m_configNSURI, "name", QString::null ) == item )
        {
            *ok = true;
            return it.text();
        }
    }
    *ok = false;
    return QString::null;
}


QString KoOasisSettings::Items::findConfigItem( const QString& item, bool* ok ) const
{
    return findConfigItem( m_element, item, ok );
}

#if 0 // does anyone need this one? passing a default value does the job, too
bool KoOasisSettings::Items::hasConfigItem( const QString& configName ) const
{
    bool ok;
    (void)findConfigItem( configName, &ok );
    return ok;
}
#endif

QString KoOasisSettings::Items::parseConfigItemString( const QString& configName, const QString& defValue ) const
{
    bool ok;
    const QString str = findConfigItem( configName, &ok );
    return ok ? str : defValue;
}

int KoOasisSettings::Items::parseConfigItemInt( const QString& configName, int defValue ) const
{
    bool ok;
    const QString str = findConfigItem( configName, &ok );
    int value;
    if ( ok ) {
        value = str.toInt( &ok );
        if ( ok )
            return value;
    }
    return defValue;
}

double KoOasisSettings::Items::parseConfigItemDouble( const QString& configName, double defValue ) const
{
    bool ok;
    const QString str = findConfigItem( configName, &ok );
    double value;
    if ( ok ) {
        value = str.toDouble( &ok );
        if ( ok )
            return value;
    }
    return defValue;
}

bool KoOasisSettings::Items::parseConfigItemBool( const QString& configName, bool defValue ) const
{
    bool ok;
    const QString str = findConfigItem( configName, &ok );
    if ( str == "true" )
        return true;
    else if ( str == "false" )
        return false;
    return defValue;
}

short KoOasisSettings::Items::parseConfigItemShort( const QString& configName, short defValue ) const
{
    bool ok;
    const QString str = findConfigItem( configName, &ok );
    short value;
    if ( ok ) {
        value = str.toShort( &ok );
        if ( ok )
            return value;
    }
    return defValue;
}

long KoOasisSettings::Items::parseConfigItemLong( const QString& configName, long defValue ) const
{
    bool ok;
    const QString str = findConfigItem( configName, &ok );
    long value;
    if ( ok ) {
        value = str.toLong( &ok );
        if ( ok )
            return value;
    }
    return defValue;
}
