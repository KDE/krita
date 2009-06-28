/* This file is part of the KDE project
 * Copyright (C) 2009 Elvis Stansvik <elvstone@gmail.org>
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

#include "TableData.h"

#include <QVector>
#include <QRectF>
#include <QTextTableCell>

TableData::TableData() : m_width(0), m_height(0)
{
}

QRectF TableData::cellContentRect(const QTextTableCell &cell) const
{
    Q_UNUSED(cell);
    return QRectF();
}

QRectF TableData::cellContentRect(int row, int column) const
{
    Q_ASSERT(row < m_rowPositions.size());
    Q_ASSERT(column < m_columnPositions.size());

    // TODO: Take borders, padding et.c. into consideration.

    return QRectF(
            m_columnPositions[column], m_rowPositions[row], // x, y
            m_columnWidths[column], m_rowHeights[row]);     // width, height
}

