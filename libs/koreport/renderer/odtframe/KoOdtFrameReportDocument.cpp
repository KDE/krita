/*
   Calligra Report Engine
   Copyright (C) 2012 by Dag Andersen (danders@get2net.dk)

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

#include "KoOdtFrameReportDocument.h"
#include "KoOdtFrameReportPrimitive.h"
#include <KoOdfWriteStore.h>
#include <KoXmlWriter.h>
#include <KoOdfGraphicStyles.h>
#include <KoGenStyles.h>
#include <KoGenStyle.h>

#include <KoStoreDevice.h>
#include <KoDpi.h>

#include "renderobjects.h"

#include <QLayout>
#include <QVarLengthArray>

#include <kdebug.h>

KoOdtFramesReportDocument::KoOdtFramesReportDocument()
    : manifestWriter(0)
{
}

KoOdtFramesReportDocument::~KoOdtFramesReportDocument()
{
    foreach (const QList<KoOdtFrameReportPrimitive*> &lst, m_pagemap) {
        foreach(KoOdtFrameReportPrimitive *p, lst) {
            delete p;
        }
    }
}

void KoOdtFramesReportDocument::setPageOptions(const ReportPageOptions &pageOptions)
{
    m_pageOptions = pageOptions;
}

void KoOdtFramesReportDocument::startTable(OROSection* section)
{
    Q_UNUSED(section);
}

void KoOdtFramesReportDocument::addPrimitive(KoOdtFrameReportPrimitive *data)
{
    m_pagemap[data->pageNumber()].append( data);
}

QFile::FileError KoOdtFramesReportDocument::saveDocument(const QString& path)
{
    // create output store
    KoStore *store = KoStore::createStore(path, KoStore::Write,
                                    "application/vnd.oasis.opendocument.text", KoStore::Zip);
    if (!store) {
        kDebug() << "Couldn't open the requested file.";
        return QFile::OpenError;
    }

    KoOdfWriteStore oasisStore(store);
    manifestWriter = oasisStore.manifestWriter("application/vnd.oasis.opendocument.text");
    if (!manifestWriter) {
        return QFile::NoError;
    }
    // save extra data like images...
    foreach (const QList<KoOdtFrameReportPrimitive*> &lst, m_pagemap) {
        foreach(KoOdtFrameReportPrimitive *p, lst) {
            p->saveData(store, manifestWriter);
        }
    }
    kDebug()<<"data saved";
    KoGenStyles coll;
    createStyles(coll); // create basic styles
    bool ok = createContent(&oasisStore, coll);
    if (ok) {
        // save styles to styles.xml
        ok = coll.saveOdfStylesDotXml(store, manifestWriter);
    }
    ok = oasisStore.closeManifestWriter() && ok;
    delete oasisStore.store();
    return ok ? QFile::NoError : QFile::WriteError;

}

void KoOdtFramesReportDocument::createStyles(KoGenStyles &coll)
{
    // convert to inches
    qreal pw = m_pageOptions.widthPx() / KoDpi::dpiX();
    qreal ph = m_pageOptions.heightPx() / KoDpi::dpiY();
    qreal topMargin = m_pageOptions.getMarginTop() / KoDpi::dpiY();
    qreal bottomMargin = m_pageOptions.getMarginBottom() / KoDpi::dpiY();
    qreal leftMargin = m_pageOptions.getMarginLeft() / KoDpi::dpiX();
    qreal rightMargin = m_pageOptions.getMarginRight() / KoDpi::dpiX();
    QString orientation = m_pageOptions.isPortrait() ? "portrait" : "landscape";

    kDebug()<<"Page:"<<pw<<ph<<orientation;
    kDebug()<<"Margin:"<<topMargin<<bottomMargin<<leftMargin<<rightMargin;

    KoGenStyle page(KoGenStyle::PageLayoutStyle, "page-layout");
    page.addProperty("style:num-format", "1");
    page.addProperty("style:print-orientation", orientation);
    page.addProperty("style:writing-mode", "lr-tb");
    page.addProperty("style:footnote-max-height", "0cm");

    page.addProperty("fo:page-width", QString("%1in").arg(pw));
    page.addProperty("fo:page-height", QString("%1in").arg(ph));
    page.addProperty("fo:margin-top", QString("%1in").arg(topMargin));
    page.addProperty("fo:margin-bottom", QString("%1in").arg(bottomMargin));
    page.addProperty("fo:margin-left", QString("%1in").arg(leftMargin));
    page.addProperty("fo:margin-right", QString("%1in").arg(rightMargin));
    page.setAutoStyleInStylesDotXml(true);
    QString pagename = coll.insert(page, "pm");

    KoGenStyle master(KoGenStyle::MasterPageStyle, "master-page");
    master.addAttribute("style:page-layout-name", pagename);
    coll.insert(master, "Standard", KoGenStyles::DontAddNumberToName);

    KoGenStyle fs(KoGenStyle::GraphicStyle, "graphic");
    fs.addProperty("vertical-pos", "from-top");
    fs.addProperty("vertical-rel", "page");
    fs.addProperty("horizontal-pos", "from-left");
    fs.addProperty("horizontal-rel", "page");
    coll.insert(fs, "Frame", KoGenStyles::DontAddNumberToName);

    KoGenStyle ps(KoGenStyle::ParagraphStyle, "paragraph");
    ps.addAttribute("style:parent-style-name", "Standard");
    coll.insert(ps, "P1", KoGenStyles::DontAddNumberToName);

}

bool KoOdtFramesReportDocument::createContent(KoOdfWriteStore* store, KoGenStyles &coll)
{
    KoXmlWriter* bodyWriter = store->bodyWriter();
    KoXmlWriter* contentWriter = store->contentWriter();

    if (!bodyWriter || !contentWriter || !manifestWriter) {
        kWarning()<<"Failed to created odt writer";
        return false;
    }

    // OpenDocument spec requires the manifest to include a list of the files in this package
    manifestWriter->addManifestEntry("content.xml",  "text/xml");
//     manifestWriter->addManifestEntry("styles.xml",  "text/xml");

    contentWriter->startElement("office:automatic-styles");

    //new page
    contentWriter->startElement("style:style");
    contentWriter->addAttribute("style:name", "NewPage");
    contentWriter->addAttribute("style:master-page-name", "Standard");
    contentWriter->addAttribute("style:family", "paragraph");
    contentWriter->startElement("style:paragraph-properties");
    contentWriter->addAttribute("fo:font-family", "Arial");
    contentWriter->addAttribute("fo:break-before", "page"); // needed by LibreOffice
    contentWriter->endElement(); // style:paragraph-properties
    contentWriter->endElement(); // style:style

    contentWriter->startElement("text:sequence-decls");
    contentWriter->startElement("text:sequence-decl");
    contentWriter->addAttribute("text:display-outline-level", "0");
    contentWriter->addAttribute("text:name", "Illustration");
    contentWriter->endElement(); //text:sequence-decl
    contentWriter->startElement("text:sequence-decl");
    contentWriter->addAttribute("text:display-outline-level", "0");
    contentWriter->addAttribute("text:name", "Table");
    contentWriter->endElement(); //text:sequence-decl
    contentWriter->startElement("text:sequence-decl");
    contentWriter->addAttribute("text:display-outline-level", "0");
    contentWriter->addAttribute("text:name", "Text");
    contentWriter->endElement(); //text:sequence-decl
    contentWriter->startElement("text:sequence-decl");
    contentWriter->addAttribute("text:display-outline-level", "0");
    contentWriter->addAttribute("text:name", "Drawing");
    contentWriter->endElement(); //text:sequence-decl
    contentWriter->endElement(); //text:sequence-decls
    contentWriter->endElement(); // office:automatic-styles

    // office:body
    bodyWriter->startElement("office:body");
    bodyWriter->startElement("office:text");

    createPages(bodyWriter, coll);

    bodyWriter->endElement();  // office:text
    bodyWriter->endElement();  // office:body

    return store->closeContentWriter();
}

void KoOdtFramesReportDocument::createPages(KoXmlWriter* bodyWriter, KoGenStyles &coll)
{
    QMap<int, QList<KoOdtFrameReportPrimitive*> >::const_iterator it;
    for (it = m_pagemap.constBegin(); it != m_pagemap.constEnd(); ++it) {
        bodyWriter->startElement("text:p");
        bodyWriter->addAttribute("text:style-name", "NewPage");
        // all frames need to be *inside* or else LibreWriter shows nothing
        foreach (KoOdtFrameReportPrimitive *data, it.value()) {
            data->createStyle(coll);
            data->createBody(bodyWriter);
        }
        bodyWriter->endElement(); // text:p
    }
    if (m_pagemap.isEmpty()) {
        // words crashes if there is no text element
        bodyWriter->startElement("text:p");
        bodyWriter->addAttribute("text:style-name", "P1");
        bodyWriter->endElement(); // text:p
    }
}
