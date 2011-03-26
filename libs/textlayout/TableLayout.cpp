/* This file is part of the KDE project
 * Copyright (C) 2009 Elvis Stansvik <elvstone@gmail.org>
 * Copyright (C) 2009 KO GmbH <cbo@kogmbh.com>
 * Copyright (C) 2010 Thomas Zander <zander@kde.org>
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

#include "TableLayout.h"
#include "TableLayoutData.h"

#include <KoTableStyle.h>
#include <KoTableBorderStyle.h>
#include <KoTableCellStyle.h>
#include <KoTableColumnStyle.h>
#include <KoTableRowStyle.h>
#include <KoTableColumnAndRowStyleManager.h>
#include <KoTextDocumentLayout.h>
#include <KoShape.h>

#include <QTextDocument>
#include <QTextTable>
#include <QTextLine>
#include <QPainter>
#include <QRectF>
#include <QPointF>

#include <kdebug.h>


QTextTableCell TableLayout::hitTestTable(const QPointF &point) const
{
    foreach (TableRect tableRect, m_tableLayoutData->m_tableRects) {
        if (tableRect.rect.contains(point)) {
            int col;
            for (col = 1; col < m_table->columns(); ++col) {
                if (point.x() < tableRect.columnPositions[col])
                    break;
            }
            --col;
            int row;
            for (row = tableRect.fromRow; row < m_table->rows(); ++row) {
                if (point.y() < m_tableLayoutData->m_rowPositions[row])
                    break;
            }
            --row;
            return m_table->cellAt(row, col);
        }
    }

    return QTextTableCell();
}

QTextTableCell TableLayout::cellAt(int position) const
{
    Q_ASSERT(isValid());

    if (!isValid())
        return QTextTableCell(); // Return invalid cell.

    return m_table->cellAt(position);
}


#include <TableLayout.moc>
