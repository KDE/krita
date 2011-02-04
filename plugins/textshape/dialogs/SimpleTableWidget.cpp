/* This file is part of the KDE project
 * Copyright (C) 2010 Casper Boemann <cbo@boemann.dk>
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
#include "SimpleTableWidget.h"
#include "TextTool.h"

#include <KAction>
#include <KDebug>

#include <QWidget>

SimpleTableWidget::SimpleTableWidget(TextTool *tool, QWidget *parent)
        : QWidget(parent),
        m_blockSignals(false),
        m_tool(tool)
{
    widget.setupUi(this);
    widget.addRowAbove->setDefaultAction(tool->action("insert_tablerow_above"));
    widget.addRowBelow->setDefaultAction(tool->action("insert_tablerow_below"));
    widget.addColumnLeft->setDefaultAction(tool->action("insert_tablecolumn_left"));
    widget.addColumnRight->setDefaultAction(tool->action("insert_tablecolumn_right"));
    widget.deleteRow->setDefaultAction(tool->action("delete_tablerow"));
    widget.deleteColumn->setDefaultAction(tool->action("delete_tablecolumn"));
    widget.mergeCells->setDefaultAction(tool->action("merge_tablecells"));
    widget.splitCells->setDefaultAction(tool->action("split_tablecells"));
}

void SimpleTableWidget::setStyleManager(KoStyleManager *sm)
{
    m_styleManager = sm;
}

#include <SimpleTableWidget.moc>
