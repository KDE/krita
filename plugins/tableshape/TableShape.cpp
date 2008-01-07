/* This file is part of the KDE project.
 *
 * Copyright (C) 2008 Boudewijn Rempt <boud@valdyas.org>
 * Copyright (C) 2008 Inge Wallin     <inge@lysator.liu.se>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

// Local
#include "TableShape.h"

// Qt
#include <QPainter>
#include <QTextDocument>
#include <QTextCursor>
#include <QTextTable>
#include <QTextTableFormat>
#include <QTextTableCell>

// KDE
#include <kdebug.h>

// KOffice
#include <KoImageData.h>
#include <KoViewConverter.h>
#include <KoImageCollection.h>
#include <KoImageData.h>
#include <KoShapeLoadingContext.h>
#include <KoOasisLoadingContext.h>
#include <KoShapeSavingContext.h>
#include <KoStoreDevice.h>
#include <KoUnit.h>
#include <KoXmlNS.h>
#include <KoXmlWriter.h>
#include <KoOdfStylesReader.h>
#include <KoTextShapeData.h>

TableShape::TableShape()
    : m_table(0)
    , m_textDocument( new QTextDocument() )
{
    
    m_textShapeData = new KoTextShapeData();
    setUserData(m_textShapeData);
}

TableShape::~TableShape() {
}

void TableShape::paint( QPainter& painter, const KoViewConverter& converter )
{
    Q_UNUSED( painter );
    Q_UNUSED( converter );
}

void TableShape::saveOdf( KoShapeSavingContext & context ) const
{
    Q_UNUSED( context );
}

bool TableShape::loadOdf( const KoXmlElement    &tableElement,
                          KoShapeLoadingContext &context )
{
    // FIXME: Get the table name (table:name)

    // Get the table style for this table. 
    // This is an attribute of table:table.
    QString  tableStyleName = tableElement.attributeNS( KoXmlNS::table,
                                                        "style-name",
                                                        "" );
    const KoXmlElement *tableStyleElement
        = context.koLoadingContext().stylesReader().findStyle( tableStyleName,
                                                               "table" );

    QString       text;
    KoXmlElement  e;
    forEachElement( e, tableElement ) {

        kDebug() << e.localName();
        
        // Only handle table rows for now.
        if ( e.localName() != "table-row" 
             || e.namespaceURI() != KoXmlNS::table )
            continue;

        // We now know it's a table row.  Loop through all the cells.
        KoXmlElement  cell;
        forEachElement( cell, e ) {
            
            // Only handle table cells for now.
            if ( cell.localName() != "table-cell" 
                 || e.namespaceURI() != KoXmlNS::table )
                continue;

            // FIXME: Get cell attribute  table:style-name
            // FIXME: Get cell attribute  office:value-type

            // FIXME: For now, support ONE text paragraph, instead of
            //        the full ODF spec.
            KoXmlElement  textElement = cell.namedItemNS( KoXmlNS::text, "p" ).toElement();
            if ( textElement.isNull() )
                continue;

            // Enter text into cell.
            KoXmlText t = textElement.toText();
            if (!t.isNull())
                text += t.data();

        } // table:table-cell

    } // table:table-row

    // Get the column styles.
    // FIXME

    // Get the rows
    
    return true;
}



