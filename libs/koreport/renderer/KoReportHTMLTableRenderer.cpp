/*
 * OpenRPT report writer and rendering engine
 * Copyright (C) 2001-2007 by OpenMFG, LLC (info@openmfg.com)
 * Copyright (C) 2007-2008 by Adam Pigg (adam@piggz.co.uk)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "KoReportHTMLTableRenderer.h"
#include "renderobjects.h"
#include "barcodes.h"
#include <kdebug.h>
#include <QDir>
#include <QPainter>
#include <QDomDocument>
#include <QRectF>
#include <QString>
#include <QFont>


#include <ktemporaryfile.h>
#include <kio/netaccess.h>
//
// KRHtmlRender
//

KoReportHTMLTableRenderer::KoReportHTMLTableRenderer()
{

}

KoReportHTMLTableRenderer::~KoReportHTMLTableRenderer()
{

}

bool KoReportHTMLTableRenderer::render(const KoReportRendererContext& context, ORODocument *document, int page)
{
    KTemporaryFile tempHtmlFile; // auto removed by default on destruction
    if (!tempHtmlFile.open()) {
        kDebug() << "Couldn't create temporary file to write into";
        return false;
    }

    QTextStream out(&tempHtmlFile);

    QString dirSuffix = ".files";
    QDir tempDir;
    QFileInfo fi(tempHtmlFile);

    QString tempFileName = fi.absoluteFilePath();
    m_tempDirName = tempFileName + dirSuffix;
    m_actualDirName = context.destinationUrl.fileName() + dirSuffix;

    if (!tempDir.mkpath(m_tempDirName))
        return false;

    out << renderTable(document);

    out.flush();
    tempHtmlFile.close();

    bool status = false;
    if (KIO::NetAccess::upload(tempFileName, context.destinationUrl, 0) && KIO::NetAccess::dircopy(KUrl(m_tempDirName),  KUrl(context.destinationUrl.url() + dirSuffix), 0)) {
        status = true;
    }

    // cleanup the temporary directory
    tempDir.setPath(m_tempDirName);
    QStringList fileList = tempDir.entryList();
    foreach(const QString& fileName, fileList) {
        tempDir.remove(fileName);
    }
    tempDir.rmdir(m_tempDirName);

    return status;
}

QString KoReportHTMLTableRenderer::renderTable(ORODocument *document)
{
    QString html;
    QString body;
    QString tr;

    bool renderedPageHeader = false;
    bool renderedPageFooter = false;

    QDir d(m_tempDirName);

    // Render Each Section
    body = "<table>\n";
    for (long s = 0; s < document->sections(); s++) {
        OROSection *section = document->section(s);
        section->sortPrimatives(OROSection::SortX);

        if (section->type() == KRSectionData::GroupHeader ||
                section->type() == KRSectionData::GroupFooter ||
                section->type() == KRSectionData::Detail ||
                section->type() == KRSectionData::ReportHeader ||
                section->type() == KRSectionData::ReportFooter ||
                (section->type() == KRSectionData::PageHeaderAny && !renderedPageHeader) ||
                (section->type() == KRSectionData::PageFooterAny && !renderedPageFooter && s > document->sections() - 2)) { //render the page foot right at the end, it will either be the last or second last section if there is a report footer
            if (section->type() == KRSectionData::PageHeaderAny)
                renderedPageHeader = true;

            if (section->type() == KRSectionData::PageFooterAny)
                renderedPageFooter = true;

            tr = "<tr style=\"background-color: " + section->backgroundColor().name() + "\">\n";
            //Render the objects in each section
            for (int i = 0; i < section->primitives(); i++) {
                OROPrimitive * prim = section->primitive(i);

                if (prim->type() == OROTextBox::TextBox) {
                    OROTextBox * tb = (OROTextBox*) prim;

                    tr += "<td>";
                    tr += tb->text();
                    tr += "</td>\n";
                } else if (prim->type() == OROImage::Image) {
                    kDebug() << "Saving an image";
                    OROImage * im = (OROImage*) prim;
                    tr += "<td>";
                    tr += "<img src=\"./" + m_actualDirName + "/object" + QString::number(s) + QString::number(i) + ".png\"></img>";
                    tr += "</td>\n";
                    im->image().save(m_tempDirName + "/object" + QString::number(s) + QString::number(i) + ".png");
                } else if (prim->type() == OROPicture::Picture) {
                    kDebug() << "Saving a picture";
                    OROPicture * im = (OROPicture*) prim;

                    tr += "<td>";
                    tr += "<img src=\"./" + m_actualDirName + "/object" + QString::number(s) + QString::number(i) + ".png\"></img>";
                    tr += "</td>\n";
                    QImage image(im->size().toSize(), QImage::Format_RGB32);
                    QPainter painter(&image);
                    im->picture()->play(&painter);
                    image.save(m_tempDirName + "/object" + QString::number(s) + QString::number(i) + ".png");
                } else {
                    kDebug() << "unhandled primitive type";
                }
            }
            tr += "</tr>\n";

            if (tr.contains("<td>")) {
                body += tr;
            }
        }
    }
    body += "</table>\n";
    html = "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01//EN\" \"http://www.w3.org/TR/html4/strict.dtd\">\n<html>\n<head>\n";
    html += "<title>" + document->title() + "</title>";
    html += "<meta name=\"generator\" content=\"Kexi - Kickass open source data management\">";
    html += "</head><body>";
    html += body;
    html += "</body></html>";

    return html;
}

