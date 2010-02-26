/*
 * Kexi Report Plugin
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
#include "KoReportKSpreadRenderer.h"
#include <kspread/part/Doc.h>
#include <kspread/Map.h>
#include <kspread/Sheet.h>
#include <KoStore.h>
#include <KoOdfWriteStore.h>
#include <KoEmbeddedDocumentSaver.h>
#include <KoDocument.h>
#include "renderobjects.h"

KoReportKSpreadRenderer::KoReportKSpreadRenderer()
{
}


KoReportKSpreadRenderer::~KoReportKSpreadRenderer()
{
}

bool KoReportKSpreadRenderer::render(const KoReportRendererContext& context, ORODocument* document, int page)
{
    KSpread::Doc *ksdoc = new KSpread::Doc();

    KSpread::Sheet *sht = ksdoc->map()->addNewSheet();

    sht->setSheetName(document->title());

    bool renderedPageHeader = false;
    bool renderedPageFooter = false;

    // Render Each Section
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

            //Render the objects in each section
            for (int i = 0; i < section->primitives(); i++) {
                OROPrimitive * prim = section->primitive(i);

                if (prim->type() == OROTextBox::TextBox) {
                    OROTextBox * tb = (OROTextBox*) prim;

                    KSpread::Cell(sht, i + 1, s + 1).parseUserInput(tb->text());
                }
                /*
                else if (prim->type() == OROImage::Image)
                {
                 kDebug() << "Saving an image";
                 OROImage * im = ( OROImage* ) prim;
                 tr += "<td>";
                 tr += "<img src=\"./" + fi.fileName() + "/object" + QString::number(s) + QString::number(i) + ".png\"></img>";
                 tr += "</td>\n";
                 im->image().save(saveDir + "/object" + QString::number(s) + QString::number(i) + ".png");
                }
                else if (prim->type() == OROPicture::Picture)
                {
                 kDebug() << "Saving a picture";
                 OROPicture * im = ( OROPicture* ) prim;

                 tr += "<td>";
                 tr += "<img src=\"./" + fi.fileName() + "/object" + QString::number(s) + QString::number(i) + ".png\"></img>";
                 tr += "</td>\n";
                 QImage image(im->size().toSize(), QImage::Format_RGB32);
                 QPainter painter(&image);
                 im->picture()->play(&painter);
                 image.save(saveDir + "/object" + QString::number(s) + QString::number(i) + ".png");
                }*/
                else {
                    kDebug() << "unhandled primitive type";
                }
            }
        }
    }

    bool status;
    if (ksdoc->exportDocument(context.destinationUrl))
        status = true;
    else
        status = false;

    delete ksdoc;
    return status;
}
