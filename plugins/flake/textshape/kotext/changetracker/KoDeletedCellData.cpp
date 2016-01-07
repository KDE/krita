/* This file is part of the KDE project
 * Copyright (C) 2011 Ganesh Paramasivam <ganesh@crystalfab.com>
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

#include "KoDeletedCellData.h"


KoDeletedCellData::KoDeletedCellData(int rowNumber, int columnNumber)
{
    this->row_number = rowNumber;
    this->column_number = columnNumber;
}

KoDeletedCellData::~KoDeletedCellData()
{
}

int KoDeletedCellData::rowNumber() const
{
    return row_number;
}

int KoDeletedCellData::columnNumber() const
{
    return column_number;
}

void KoDeletedCellData::setCellFormat(const QTextTableCellFormat &cellFormat)
{
    this->cell_format = cellFormat;
}

const QTextTableCellFormat& KoDeletedCellData::cellFormat() const
{
    return this->cell_format;
}

void KoDeletedCellData::setCellContent(const QTextDocumentFragment &cellContent)
{
    this->cell_content = cellContent;
}

const QTextDocumentFragment& KoDeletedCellData::cellContent() const
{
    return this->cell_content;
}

