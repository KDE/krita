/*
   KoReport Library
   Copyright (C) 2010 by Adam Pigg (adam@piggz.co.uk)

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "KoSimpleOdsDocument.h"
#include <kdebug.h>
#include <KoOdfWriteStore.h>
#include <KoXmlWriter.h>
#include "KoSimpleOdsSheet.h"

KoSimpleOdsDocument::KoSimpleOdsDocument()
{
    m_store = 0;
}

KoSimpleOdsDocument::~KoSimpleOdsDocument()
{
    delete m_store;
    qDeleteAll(m_worksheets);
}

void KoSimpleOdsDocument::addSheet(KoSimpleOdsSheet* sheet)
{
    if (!m_worksheets.contains(sheet)) {
        m_worksheets.append(sheet);
    }
}

QFile::FileError KoSimpleOdsDocument::saveDocument(const QString& path)
{
    // create output store
    delete m_store;
    m_store = KoStore::createStore(path, KoStore::Write,
                                    "application/vnd.oasis.opendocument.spreadsheet", KoStore::Zip);
    if (!m_store) {
        kDebug() << "Couldn't open the requested file.";
        return QFile::OpenError;
    }

    KoOdfWriteStore oasisStore(m_store);
    //KoXmlWriter* manifestWriter = oasisStore.manifestWriter("application/vnd.oasis.opendocument.spreadsheet");

    if (!createContent(&oasisStore)) {
        delete m_store;
	m_store = 0;
        return QFile::WriteError;
    }

    delete m_store;
    m_store = 0;
    return QFile::NoError;

}

// Writes the spreadsheet content into the content.xml
bool KoSimpleOdsDocument::createContent(KoOdfWriteStore* store)
{
    KoXmlWriter* bodyWriter = store->bodyWriter();
    KoXmlWriter* contentWriter = store->contentWriter();
    KoXmlWriter* manifestWriter = store->manifestWriter("application/vnd.oasis.opendocument.spreadsheet");

    if (!bodyWriter || !contentWriter || !manifestWriter) {
        kDebug() << "Bad things happened";
        return false;
    }

    // OpenDocument spec requires the manifest to include a list of the files in this package
    manifestWriter->addManifestEntry("content.xml",  "text/xml");

    // FIXME this is dummy and hardcoded, replace with real font names
    contentWriter->startElement("office:font-face-decls");
    contentWriter->startElement("style:font-face");
    contentWriter->addAttribute("style:name", "Arial");
    contentWriter->addAttribute("svg:font-family", "Arial");
    contentWriter->endElement(); // style:font-face
    contentWriter->startElement("style:font-face");
    contentWriter->addAttribute("style:name", "Times New Roman");
    contentWriter->addAttribute("svg:font-family", "&apos;Times New Roman&apos;");
    contentWriter->endElement(); // style:font-face
    contentWriter->endElement(); // office:font-face-decls

     // office:body
    bodyWriter->startElement("office:body");
    foreach(KoSimpleOdsSheet *sheet, m_worksheets) {
        bodyWriter->startElement("office:spreadsheet");
        sheet->saveSheet(bodyWriter);
        bodyWriter->endElement();
    }
    bodyWriter->endElement();  // office:body

    return store->closeContentWriter() && store->closeManifestWriter();
}
