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

#include "ListStyleButton.h"

#include "ListItemsHelper.h"

#include <QMenu>

ListStyleButton::ListStyleButton( QWidget *parent)
    : QToolButton(parent)
    , m_letterSynchronization(false)
    , m_lastAction(0)
{
    m_menu = new QMenu();
    setPopupMode(MenuButtonPopup);
    setMenu(m_menu);
    connect(this, SIGNAL(triggered(QAction *)), this, SLOT(itemSelected(QAction *)));
}

void ListStyleButton::addItem(QPixmap pm, int id)
{
    m_actionMap[m_menu->addAction(QIcon(pm), QString())] = id;
    if (!m_lastAction) {
        m_lastAction = m_actionMap.key(id);
    }
}

void ListStyleButton::itemSelected(QAction *action)
{
    if(action != defaultAction()) {
        m_lastAction = action;
    }
    emit itemTriggered(m_actionMap[m_lastAction]);
}

QString ListStyleButton::example(KoListStyle::Style type) const {
    int value=1;
    switch( type ) {
        case KoListStyle::DecimalItem:
            return QString::number(value);
        case KoListStyle::AlphaLowerItem:
            return Lists::intToAlpha(value, Lists::Lowercase, m_letterSynchronization);
        case KoListStyle::UpperAlphaItem:
            return Lists::intToAlpha(value, Lists::Uppercase, m_letterSynchronization);
        case KoListStyle::RomanLowerItem:
            return Lists::intToRoman(value);
        case KoListStyle::UpperRomanItem:
            return Lists::intToRoman(value).toUpper();
        case KoListStyle::Bengali:
        case KoListStyle::Gujarati:
        case KoListStyle::Gurumukhi:
        case KoListStyle::Kannada:
        case KoListStyle::Malayalam:
        case KoListStyle::Oriya:
        case KoListStyle::Tamil:
        case KoListStyle::Telugu:
        case KoListStyle::Tibetan:
        case KoListStyle::Thai:
            return Lists::intToScript(value, type);
        case KoListStyle::Abjad:
        case KoListStyle::ArabicAlphabet:
        case KoListStyle::AbjadMinor:
            return Lists::intToScriptList(value, type);
        default:  // others we ignore.
            return "hmmX";
    }
}

#include <ListStyleButton.moc>
