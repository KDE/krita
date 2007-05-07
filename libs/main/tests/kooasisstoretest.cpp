/* This file is part of the KDE project
   Copyright (C) 2005 David Faure <faure@kde.org>

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
#include <KoOasisStore.h>
#include <KoDom.h>
#include <kdebug.h>
#include <assert.h>
//Added by qt3to4:
#include <Q3CString>

void testMimeForPath( KoXmlDocument& doc )
{
    QString mime = KoOasisStore::mimeForPath( doc, "Object 1" );
    kDebug() << k_funcinfo << mime << endl;
    assert( !mime.isNull() );
    assert( !mime.isEmpty() );
    assert( mime == "application/vnd.oasis.opendocument.text" );
    kDebug() << "testMimeForPath OK" << endl;
}

int main( int, char** ) {

    const Q3CString xml = "\
<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n\
<manifest:manifest xmlns:manifest=\"urn:oasis:names:tc:opendocument:xmlns:manifest:1.0\">\n\
 <manifest:file-entry manifest:media-type=\"application/vnd.oasis.opendocument.text\" manifest:full-path=\"/\"/>\n\
 <manifest:file-entry manifest:media-type=\"text/xml\" manifest:full-path=\"content.xml\"/>\n\
 <manifest:file-entry manifest:media-type=\"application/vnd.oasis.opendocument.text\" manifest:full-path=\"Object 1\"/>\n\
</manifest:manifest> \
";

    KoXmlDocument doc;
    QString errorMsg;
    int errorLine, errorColumn;
    bool ok = doc.setContent( xml, true /* namespace processing */, &errorMsg, &errorLine, &errorColumn );
    if ( !ok ) {
        kError() << "Parsing error! Aborting!" << endl
            << " In line: " << errorLine << ", column: " << errorColumn << endl
            << " Error message: " << errorMsg << endl;
        return 1;
    }

    testMimeForPath( doc );
    return 0;
}
