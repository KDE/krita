/* This file is part of the KDE project
   Copyright (C) 2004 David Faure <faure@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include <KoXmlReader.h>
#include <KoOasisSettings.h>
#include <KoDom.h>
#include <kdebug.h>
#include <assert.h>
//Added by qt3to4:
#include <Q3CString>

void testSelectItemSet( KoOasisSettings& settings )
{
    KoOasisSettings::Items items = settings.itemSet( "notexist" );
    assert( items.isNull() );
    items = settings.itemSet( "view-settings" );
    assert( !items.isNull() );
    kDebug() << "testSelectItemSet OK" << endl;
}

void testParseConfigItemString( KoOasisSettings& settings )
{
    KoOasisSettings::Items viewSettings = settings.itemSet( "view-settings" );
    const QString unit = viewSettings.parseConfigItemString( "unit" );
    qDebug( "%s", qPrintable( unit ) );
    assert( unit == "mm" );
    kDebug() << "testParseConfigItemString OK" << endl;
}

void testIndexedMap( KoOasisSettings& settings )
{
    KoOasisSettings::Items viewSettings = settings.itemSet( "view-settings" );
    assert( !viewSettings.isNull() );
    KoOasisSettings::IndexedMap viewMap = viewSettings.indexedMap( "Views" );
    assert( !viewMap.isNull() );
    KoOasisSettings::Items firstView = viewMap.entry( 0 );
    assert( !firstView.isNull() );
    const short zoomFactor = firstView.parseConfigItemShort( "ZoomFactor" );
    assert( zoomFactor == 100 );
    KoOasisSettings::Items secondView = viewMap.entry( 1 );
    assert( secondView.isNull() );
    kDebug() << "testIndexedMap OK" << endl;
}

void testNamedMap( KoOasisSettings& settings )
{
    KoOasisSettings::Items viewSettings = settings.itemSet( "view-settings" );
    assert( !viewSettings.isNull() );
    KoOasisSettings::NamedMap viewMap = viewSettings.namedMap( "NamedMap" );
    assert( !viewMap.isNull() );
    KoOasisSettings::Items foo = viewMap.entry( "foo" );
    assert( !foo.isNull() );
    const int zoomFactor = foo.parseConfigItemShort( "ZoomFactor" );
    assert( zoomFactor == 100 );
    KoOasisSettings::Items secondView = viewMap.entry( "foobar" );
    assert( secondView.isNull() );
    kDebug() << "testNamedMap OK" << endl;
}

int main( int, char** ) {

    const Q3CString xml = "\
<?xml version=\"1.0\" encoding=\"UTF-8\"?> \
<office:document-settings xmlns:office=\"urn:oasis:names:tc:opendocument:xmlns:office:1.0\" xmlns:config=\"urn:oasis:names:tc:opendocument:xmlns:config:1.0\"> \
 <office:settings> \
  <config:config-item-set config:name=\"view-settings\"> \
    <config:config-item config:name=\"unit\" config:type=\"string\">mm</config:config-item> \
    <config:config-item-map-indexed config:name=\"Views\"> \
      <config:config-item-map-entry> \
        <config:config-item config:name=\"ZoomFactor\" config:type=\"short\">100</config:config-item> \
      </config:config-item-map-entry> \
    </config:config-item-map-indexed> \
    <config:config-item-map-named config:name=\"NamedMap\"> \
      <config:config-item-map-entry config:name=\"foo\"> \
        <config:config-item config:name=\"ZoomFactor\" config:type=\"int\">100</config:config-item> \
      </config:config-item-map-entry> \
    </config:config-item-map-named> \
  </config:config-item-set> \
 </office:settings> \
</office:document-settings> \
";

    KoXmlDocument doc;
    bool ok = doc.setContent( xml, true /* namespace processing */ );
    assert( ok );

    KoOasisSettings settings( doc );

    testSelectItemSet( settings );
    testParseConfigItemString( settings );
    testIndexedMap( settings );
    testNamedMap( settings );
    return 0;
}
