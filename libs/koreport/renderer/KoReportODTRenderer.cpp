/*
 * Kexi Report Plugin
 * Copyright (C) 2010 by Adam Pigg (adam@piggz.co.uk)
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

#include "KoReportODTRenderer.h"
#include <QTextDocument>
#include <QTextTable>
#include <QTextTableFormat>
#include <QPainter>
#include <QTextDocumentWriter>
#include "renderobjects.h"
#include <kdebug.h>

KoReportODTRenderer::KoReportODTRenderer() : m_document(new QTextDocument()), m_cursor(m_document)
{

}

KoReportODTRenderer::~KoReportODTRenderer()
{
    delete m_document;
}

bool KoReportODTRenderer::render(const KoReportRendererContext& context, ORODocument* document, int /*page*/)
{
    QTextTableFormat tableFormat;
    tableFormat.setCellPadding(5);
    tableFormat.setHeaderRowCount(1);
    tableFormat.setBorderStyle(QTextFrameFormat::BorderStyle_Solid);
    tableFormat.setWidth(QTextLength(QTextLength::PercentageLength, 100));
    QTextTable *table = m_cursor.insertTable(1, 1, tableFormat);

    long renderedSections = 0;

    for (long s = 0; s < document->sections(); s++) {
        OROSection *section = document->section(s);
        section->sortPrimatives(OROSection::SortX);

        if (section->type() == KRSectionData::GroupHeader || section->type() == KRSectionData::GroupFooter ||
            section->type() == KRSectionData::ReportHeader || section->type() == KRSectionData::ReportFooter ||
            section->type() == KRSectionData::Detail){
            //Add this section to the document

            //Resize the table to accommodate all the primitives in the section
            if (table->columns() < section->primitives()) {
                table->appendColumns(section->primitives() - table->columns());
            }

            if (renderedSections > 0) {
                //We need to back a row, then forward a row to get at the start cell
                m_cursor.movePosition(QTextCursor::PreviousRow);
                m_cursor.movePosition(QTextCursor::NextRow);
            } else {
                //On the first row, ensure we are in the first cell after expanding the table
                while (m_cursor.movePosition(QTextCursor::PreviousCell)){}
            }
            //Render the objects in each section
            for (int i = 0; i < section->primitives(); i++) {
                //Colour the cell using hte section background colour
                OROPrimitive * prim = section->primitive(i);
                QTextTableCell cell = table->cellAt(m_cursor);
                QTextCharFormat format = cell.format();
                format.setBackground(section->backgroundColor());
                cell.setFormat(format);

                if (prim->type() == OROTextBox::TextBox) {
                    OROTextBox * tb = (OROTextBox*) prim;
                    m_cursor.insertText(tb->text());
                } else if (prim->type() == OROImage::Image) {
                    OROImage * im = (OROImage*) prim;

                    m_cursor.insertImage(im->image().scaled(im->size().width(), im->size().height(), Qt::KeepAspectRatio));

                } else if (prim->type() == OROPicture::Picture) {
                    OROPicture * im = (OROPicture*) prim;

                    QImage image(im->size().toSize(), QImage::Format_RGB32);
                    QPainter painter(&image);
                    im->picture()->play(&painter);


                    m_cursor.insertImage(image);
                } else {
                    kDebug() << "unhandled primitive type";
                }
                m_cursor.movePosition(QTextCursor::NextCell);

            }
            if (s < document->sections() - 1) {
                table->appendRows(1);
            }

            renderedSections++;
        }
    }

    QTextDocumentWriter writer(context.destinationUrl.toLocalFile());
    return writer.write(m_document);
}

