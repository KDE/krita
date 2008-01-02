/* This file is part of the KDE project
 * Copyright (C) 2008 Boudewijn Rempt <boud@valdyas.org>
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

#include "TableShape.h"

#include <KoImageData.h>
#include <KoViewConverter.h>
#include <KoImageCollection.h>
#include <KoImageData.h>
#include <KoShapeLoadingContext.h>
#include <KoOasisLoadingContext.h>
#include <KoShapeSavingContext.h>
#include <KoXmlWriter.h>
#include <KoStoreDevice.h>
#include <KoUnit.h>

#include <QPainter>
#include <kdebug.h>

#include <QTextDocument>
#include <QTextCursor>
#include <QTextTable>
#include <QTextTableFormat>
#include <QTextTableCell>

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
    : m_textDocument( new QTextDocument() )
    , m_table(0)
{
    createExampleData();
}

TableShape::~TableShape() {
}

void TableShape::paint( QPainter& painter, const KoViewConverter& converter )
{
}

void TableShape::saveOdf( KoShapeSavingContext & context ) const
{
}

bool TableShape::loadOdf( const KoXmlElement & element, KoShapeLoadingContext &context )
{
    
    return true;
}

void TableShape::createExampleData()
{
    const int ROWS = 10;
    const int COLUMNS = 3;
    QTextTableFormat tableFormat;
    QTextCursor cursor(m_textDocument);
    cursor.movePosition(QTextCursor::Start);
    QTextTable m_table = cursor.insertTable(ROWS, COLUMNS, tableFormat);
    
    QTextTableCell curCell = table->cellAt(0,0);
    
    qDebug() << "Is the first cursor pos in the table also the first cursor pos in the document? " << cursor.position() << ", " << curCell.firstCursorPosition().position();
    
    cursor = curCell.firstCursorPosition();
    QTextCharFormat charFormat;
    charFormat->setFont(QFont("Times", 10, QFont::Bold));
    cursor.insertText("Hello World");
    cursor = curCell.firstCursorPosition();
    cursor.select(QTextCursor::LineUnderCursor);
    cursor->setCharFormat(charFormat);

    charFormat->setFont(QFont("Helvetica", 20, QFont::Italic));
    curCell = table->cellAt(0,2);
    cursor = curCell.firstCursorPosition();
    cursor.insertText("Bonjour Monde!");
    cursor = curCell.firstCursorPosition();
    cursor.select(QTextCursor::LineUnderCursor);
    cursor->setCharFormat(charFormat);
    
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

void TableShape::recalculateCellHeights(int row, int column)
{
}

void TableShape::initTableFrames()
{
    if (!m_table) return;

    QSize shapeSize = size();
    float widthInPoints = shapeSize.width() / m_table->columns();
    m_tableFrames.resize(m_table.rows());
    
    for (int row = 0; row < m_table->rows(); ++row) {
        m_tableFrames[row].resize(m_table.columns());
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