/* This file is part of the KDE project
* Copyright (C) 2009 Pierre Stirnweiss <pstirnweiss@googlemail.com>
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

#include "TrackedChangeManager.h"

#include "TrackedChangeModel.h"

#include <KLocale>

#include <QModelIndex>
#include <QTreeView>

#include <KDebug>

TrackedChangeManager::TrackedChangeManager(QWidget* parent): QWidget(parent),
    m_model(0)
{
    widget.setupUi(this);
}

TrackedChangeManager::~TrackedChangeManager()
{
}

void TrackedChangeManager::setModel(TrackedChangeModel* model)
{
    m_model = model;
    widget.treeView->setModel(m_model);
    widget.treeView->reset();
    connect(widget.treeView->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)), this, SLOT(currentChanged(QModelIndex,QModelIndex)));
}

void TrackedChangeManager::currentChanged(QModelIndex newIndex, QModelIndex previousIndex)
{
    Q_UNUSED(previousIndex);
    emit currentChanged(newIndex);
}

void TrackedChangeManager::selectItem(QModelIndex newIndex)
{
    QModelIndex currentIndex = widget.treeView->currentIndex();
    widget.treeView->setCurrentIndex(newIndex);
    currentChanged(newIndex, currentIndex);
}

#include <TrackedChangeManager.moc>
