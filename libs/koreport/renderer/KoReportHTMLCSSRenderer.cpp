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

#include "KoReportHTMLCSSRenderer.h"
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

KoReportHTMLCSSRenderer::KoReportHTMLCSSRenderer()
{

}

KoReportHTMLCSSRenderer::~KoReportHTMLCSSRenderer()
{

}

bool KoReportHTMLCSSRenderer::render(const KoReportRendererContext& context, ORODocument *document, int page)
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

    out << renderCSS(document);

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

QString KoReportHTMLCSSRenderer::renderCSS(ORODocument *document)
{
    QString html;
    QString body;
    QString style;
    QStringList styles;
    int styleindex;
    bool renderedPageHead = false;
    bool renderedPageFoot = false;

    kDebug() << "4";

    QDir d(m_tempDirName);
    // Render Each Section
    for (long s = 0; s < document->sections(); s++) {
        OROSection *section = document->section(s);

        if (section->type() == KRSectionData::GroupHeader ||
                section->type() == KRSectionData::GroupFooter ||
                section->type() == KRSectionData::Detail ||
                section->type() == KRSectionData::ReportHeader ||
                section->type() == KRSectionData::ReportFooter ||
                (section->type() == KRSectionData::PageHeaderAny && !renderedPageHead) ||
                (section->type() == KRSectionData::PageFooterAny && !renderedPageFoot && s > document->sections() - 2)) { //render the page foot right at the end, it will either be the last or second last section if there is a report footer
            if (section->type() == KRSectionData::PageHeaderAny)
                renderedPageHead = true;

            if (section->type() == KRSectionData::PageFooterAny)
                renderedPageFoot = true;

            style = "position: relative; top: 0pt; left: 0pt; background-color: " + section->backgroundColor().name() + "; height: " + QString::number(section->height()) + "pt;";

            if (!styles.contains(style)) {
                styles << style;
            }
            styleindex = styles.indexOf(style);

            body += "<div class=\"style" + QString::number(styleindex) + "\">\n";
            //Render the objects in each section
            for (int i = 0; i < section->primitives(); i++) {
                OROPrimitive * prim = section->primitive(i);
                kDebug() << "Got object type" << prim->type();
                if (prim->type() == OROTextBox::TextBox) {
                    OROTextBox * tb = (OROTextBox*) prim;

                    style = "position: absolute; ";
                    style += "background-color: " + (tb->textStyle().backgroundOpacity == 0 ? "transparent" : tb->textStyle().backgroundColor.name()) + "; ";
                    style += "top: " + QString::number(tb->position().y()) + "pt; ";
                    style += "left: " + QString::number(tb->position().x()) + "pt; ";
                    style += "font-size: " + QString::number(tb->textStyle().font.pointSize()) + "pt; ";
                    style += "color: " + tb->textStyle().foregroundColor.name() + "; ";
                    //TODO opaque text + translucent background
                    //it looks a pain to implement
                    //http://developer.mozilla.org/en/docs/Useful_CSS_tips:Color_and_Background
                    //style += "filter:alpha(opacity=" + QString::number((tb->textStyle().bgOpacity / 255) * 100) + ");"; //ie opacity
                    //style += "opacity: " + QString::number(tb->textStyle().bgOpacity / 255.0) + ";";

                    if (!styles.contains(style)) {
                        styles << style;
                    }
                    styleindex = styles.indexOf(style);

                    body += "<div class=\"style" + QString::number(styleindex) + "\">";
                    body += tb->text();
                    body += "</div>\n";
                } else if (prim->type() == OROImage::Image) {
                    kDebug() << "Saving an image";
                    OROImage * im = (OROImage*) prim;
                    style = "position: absolute; ";
                    style += "top: " + QString::number(im->position().y()) + "pt; ";
                    style += "left: " + QString::number(im->position().x()) + "pt; ";
                    if (!styles.contains(style)) {
                        styles << style;
                    }
                    styleindex = styles.indexOf(style);

                    body += "<div class=\"style" + QString::number(styleindex) + "\">";
                    body += "<img width=\"" + QString::number(im->size().width()) + "px" + "\" height=\"" + QString::number(im->size().height()) + "px" + "\" src=\"./" + m_actualDirName + "/object" + QString::number(s) + QString::number(i) + ".png\"></img>";
                    body += "</div>\n";


                    im->image().save(m_tempDirName + "/object" + QString::number(s) + QString::number(i) + ".png");
                } else if (prim->type() == OROPicture::Picture) {
                    kDebug() << "Saving a picture";
                    OROPicture * im = (OROPicture*) prim;
                    style = "position: absolute; ";
                    style += "top: " + QString::number(im->position().y()) + "pt; ";
                    style += "left: " + QString::number(im->position().x()) + "pt; ";
                    if (!styles.contains(style)) {
                        styles << style;
                    }
                    styleindex = styles.indexOf(style);

                    body += "<div class=\"style" + QString::number(styleindex) + "\">";
                    body += "<img width=\"" + QString::number(im->size().width()) + "px" + "\" height=\"" + QString::number(im->size().height()) + "px" + "\" src=\"./" + m_actualDirName + "/object" + QString::number(s) + QString::number(i) + ".png\"></img>";
                    body += "</div>\n";

                    QImage image(im->size().toSize(), QImage::Format_RGB32);
                    QPainter painter(&image);
                    im->picture()->play(&painter);
                    image.save(m_tempDirName + "/object" + QString::number(s) + QString::number(i) + ".png");
                } else {
                    kDebug() << "unrecognized primitive type" << prim->type();
                }
            }
            body += "</div>\n";
        }
    }

    html = "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01//EN\" \"http://www.w3.org/TR/html4/strict.dtd\">\n<html>\n<head>\n";
    html += "<style type=\"text/css\">";

    for (int i = 0; i < styles.count(); ++i) {
        html += ".style" + QString::number(i) + '{' + styles[i] + "}\n";
    }

    html += "</style>";
    html += "<title>" + document->title() + "</title>";
    html += "<meta name=\"generator\" content=\"Kexi - Kickass open source data management\">";
    html += "</head><body>";
    html += body;
    html += "</body></html>";

    return html;
}
