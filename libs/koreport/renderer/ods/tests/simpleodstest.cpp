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
#include <kurl.h>
#include <QCoreApplication>
#include <QDir>

//Test ods writing

int main(int argc, char** argv)
{
    QCoreApplication app(argc, argv);
    KoSimpleOdsDocument ods;

    KoSimpleOdsSheet *sheet = new KoSimpleOdsSheet;
    sheet->setName("Report");

    sheet->addCell(0,0,new KoSimpleOdsCell("The"));
    sheet->addCell(1,1,new KoSimpleOdsCell("Quick"));
    sheet->addCell(2,2,new KoSimpleOdsCell("Brown"));
    sheet->addCell(3,3,new KoSimpleOdsCell("Fox"));
    sheet->addCell(1,4,new KoSimpleOdsCell("1"));
    sheet->addCell(2,5,new KoSimpleOdsCell("10"));
    sheet->addCell(3,6,new KoSimpleOdsCell("100"));
    
    ods.addSheet(sheet);
    
    ods.saveDocument(QDir::homePath() + "/simpleodstestfile.ods");
    return 0;
}
