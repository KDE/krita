/* This file is part of the KDE project
 * Copyright (C) 2007 Marijn Kruisselbrink <m.kruisselbrink@student.tue.nl>
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
#include "PartsWidget.h"
#include "PartDetailsDialog.h"
#include "PartsListModel.h"

#include "../MusicTool.h"
#include "../MusicShape.h"

#include "../core/Sheet.h"
#include "../core/Part.h"

#include "../commands/RemovePartCommand.h"
#include "../commands/AddPartCommand.h"

#include <KIcon>

using namespace MusicCore;

PartsWidget::PartsWidget(MusicTool *tool, QWidget *parent)
    : QWidget(parent),
    m_tool(tool)
{
    widget.setupUi(this);

    widget.addPart->setIcon(KIcon("list-add"));
    widget.removePart->setIcon(KIcon("list-remove"));
    widget.editPart->setIcon(KIcon("document-properties"));

    connect(widget.partsList, SIGNAL(doubleClicked(const QModelIndex&)), this, SLOT(partDoubleClicked(const QModelIndex&)));
    //connect(widget.partsList, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)), this, SLOT(selectionChanged(QListWidgetItem*,QListWidgetItem*)));
    connect(widget.addPart, SIGNAL(clicked()), this, SLOT(addPart()));
    connect(widget.removePart, SIGNAL(clicked()), this, SLOT(removePart()));
    connect(widget.editPart, SIGNAL(clicked()), this, SLOT(editPart()));
}

void PartsWidget::setShape(MusicShape* shape)
{
    Sheet* sheet = shape->sheet();
    m_shape = shape;
//    widget.partsList->clear();
//    for (int i = 0; i < sheet->partCount(); i++) {
//        widget.partsList->addItem(sheet->part(i)->name());
//    }
    widget.partsList->setModel(new PartsListModel(sheet));
    connect(widget.partsList->selectionModel(), SIGNAL(currentRowChanged(const QModelIndex&, const QModelIndex&)), this, SLOT(selectionChanged(const QModelIndex&, const QModelIndex&)));
    m_sheet = sheet;
}

void PartsWidget::partDoubleClicked(const QModelIndex & index)
{
    int row = index.row();
    PartDetailsDialog::showDialog(m_tool, m_sheet->part(row), this);
}

void PartsWidget::selectionChanged(const QModelIndex& current, const QModelIndex& prev)
{
    Q_UNUSED( prev );
    widget.editPart->setEnabled(current.isValid());
    widget.removePart->setEnabled(current.isValid());
}

void PartsWidget::addPart()
{
    m_tool->addCommand(new AddPartCommand(m_shape));
}

void PartsWidget::removePart()
{
    Part* part = m_sheet->part(widget.partsList->currentIndex().row());

    m_tool->addCommand(new RemovePartCommand(m_shape, part));
}

void PartsWidget::editPart()
{
    int row = widget.partsList->currentIndex().row();
    PartDetailsDialog::showDialog(m_tool, m_sheet->part(row), this);
}

#include <PartsWidget.moc>
