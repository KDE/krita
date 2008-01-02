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

TableShape::TableShape()
    : m_textDocument( new QTextDocument() )
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
    QTextTable *table = cursor.insertTable(ROWS, COLUMNS, tableFormat);
    
    QTextTableCell curCell = table->cellAt(0,0);
    
    qDebug() << "Is the first cursor pos in the table also the first cursor pos in the document? " << cursor.position() << ", " << curCell.firstCursorPosition().position();
    
    cursor = curCell.firstCursorPosition();
    cursor.insertText("Hello World");

    curCell = table->cellAt(0,2);
    cursor = curCell.firstCursorPosition();
    cursor.insertText("Bonjour Monde!");
}
