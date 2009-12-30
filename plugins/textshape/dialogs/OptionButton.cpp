/* This file is part of the KDE project
 * Copyright (C) 2007 Thomas Zander <zander@kde.org>
 * Copyright (C) 2009 Casper Boemann <cbo@boemann.dk>
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

#include "OptionButton.h"

#include "ListItemsHelper.h"

#include <QMenu>

OptionButton::OptionButton( QWidget *parent)
    : QToolButton(parent)
    , m_firstItemAdded(false)
{
    m_menu = new QMenu();
    setPopupMode(MenuButtonPopup);
    setMenu(m_menu);
    connect(this, SIGNAL(triggered(QAction *)), this, SLOT(itemSelected(QAction *)));
}

void OptionButton::addItem(QPixmap pm, int id)
{
    m_actionMap[m_menu->addAction(QIcon(pm),"")] = id;
    if(!m_firstItemAdded) {
        setDefaultAction(m_actionMap.key(id));
        setIcon(QIcon(pm));
    }
    m_firstItemAdded=true;
}

void OptionButton::itemSelected(QAction *action)
{
    setDefaultAction(action);
    setIcon(action->icon());
    emit itemTriggered(m_actionMap[action]);
}

#include <OptionButton.moc>
