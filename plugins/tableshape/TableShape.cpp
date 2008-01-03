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


/**
 * The size of a layouted cell in Postscript points. We're doing row-first,
 * so, cell heights change with content, but not width.
 */
struct TableShape::TableCellFrame {
    double width;    // width as set by the document/user
    double height;   // height as determined by the number of lines of text
    double spacing; // space between cells
    double padding; // space between contents and cell border
};


TableShape::TableShape()
    : m_table(0)
    , m_textDocument( new QTextDocument() )
{
    createExampleData();
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

void TableShape::createExampleData()
{
    const int ROWS = 10;
    const int COLUMNS = 3;
    QTextTableFormat tableFormat;
    QTextCursor cursor(m_textDocument);
    cursor.movePosition(QTextCursor::Start);
    m_table = cursor.insertTable(ROWS, COLUMNS, tableFormat);
    
    QTextTableCell curCell = m_table->cellAt(0,0);
    
    qDebug() << "Is the first cursor pos in the table also the first cursor pos in the document? " << cursor.position() << ", " << curCell.firstCursorPosition().position();
    
    cursor = curCell.firstCursorPosition();
    QTextCharFormat charFormat;
    charFormat.setFont(QFont("Times", 10, QFont::Bold));
    cursor.insertText("Hello World");
    cursor = curCell.firstCursorPosition();
    cursor.select(QTextCursor::LineUnderCursor);
    cursor.setCharFormat(charFormat);

    charFormat.setFont(QFont("Helvetica", 20, QFont::Normal, true)); // italic
    curCell = m_table->cellAt(0,2);
    cursor = curCell.firstCursorPosition();
    cursor.insertText("Bonjour Monde!");
    cursor = curCell.firstCursorPosition();
    cursor.select(QTextCursor::LineUnderCursor);
    cursor.setCharFormat(charFormat);
    
    initTableFrames();
}

void TableShape::recalculateCellPositions()
{
    if (!m_table) return;
    
    for (int row = 0; row < m_table->rows(); ++row) {
        double maxHeight = 0;
        for (int column = 0; column < m_table->columns(); ++column) {
            double height = recalculateCellHeight(row, column);
            if (height > maxHeight) maxHeight = height;
        }
        for (int column = 0; column < m_table->columns(); ++column) {
            
        }
    }
}

double TableShape::recalculateCellHeight(int row, int column)
{
    Q_UNUSED( row );
    Q_UNUSED( column );

    // FIXME
    return 10.0;
}

void TableShape::initTableFrames()
{
    if (!m_table) return;

    QSize shapeSize = QSize();
    float widthInPoints = shapeSize.width() / m_table->columns();
    m_tableFrames.resize( m_table->rows() );
    
    for (int row = 0; row < m_table->rows(); ++row) {
        m_tableFrames[row].resize(m_table->columns());
        for (int column = 0; column < m_table->columns(); ++column) {
            TableCellFrame * frame = new TableCellFrame();
            frame->height = 0;
            frame->width = widthInPoints;
            frame->spacing = 0;
            frame->padding = 1;
            m_tableFrames[row][column] = frame;
        }
    }
}
