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

#include "KoSimpleOdsSheet.h"
#include <KoXmlWriter.h>
#include <kdebug.h>

KoSimpleOdsSheet::KoSimpleOdsSheet()
{

}

KoSimpleOdsSheet::~KoSimpleOdsSheet()
{

}

void KoSimpleOdsSheet::addCell(long row, long col, KoSimpleOdsCell *cell)
{
    m_model.setItem(row, col, cell);
}

void KoSimpleOdsSheet::setName(const QString& name)
{
    m_name = name;
}

void KoSimpleOdsSheet::saveSheet(KoXmlWriter *writer)
{
    writer->startElement("table:table");
    writer->addAttribute("table:name", (m_name.isEmpty() ? "Sheet" : m_name ));
    //Add table:style-name ?

    for (long r = 0; r < m_model.rowCount(); ++r) {
        writer->startElement("table:table-row");
        for (long c = 0; c < m_model.columnCount(); ++c) {
            kDebug() << ".";
            
            KoSimpleOdsCell *cell = dynamic_cast<KoSimpleOdsCell*>(m_model.item(r,c));
            if (cell) {
                cell->writeCellData(writer);
            } else {
                KoSimpleOdsCell::writeCellData(writer, QString());
            }
    }
        writer->endElement();
    }
    writer->endElement();
}
