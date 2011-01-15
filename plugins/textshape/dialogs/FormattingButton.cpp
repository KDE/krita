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

#include "FormattingButton.h"

#include "ListItemsHelper.h"

#include <QMenu>
#include <QFrame>
#include <QGridLayout>
#include <QWidgetAction>

#include <kdebug.h>

//This class is the main place where the expanding grid is done
class ItemChooserAction : public QWidgetAction
{
public:
    ItemChooserAction();
    QWidget *m_widget;
    QGridLayout *m_containerLayout;
    int m_cnt;
    QToolButton *addItem(QPixmap pm);
};

ItemChooserAction::ItemChooserAction()
 : QWidgetAction(0)
 , m_cnt(0)
{
    m_widget = new QWidget;
    m_containerLayout = new QGridLayout();
    m_containerLayout->setSpacing(6);
    m_widget->setLayout(m_containerLayout);
    setDefaultWidget(m_widget);
    m_widget->setBackgroundRole(QPalette::Base);
}

QToolButton *ItemChooserAction::addItem(QPixmap pm)
{
    QToolButton *b = new QToolButton();
    b->setIcon(QIcon(pm));
    b->setIconSize(QSize(48,48));
    b->setAutoRaise(true);
    m_containerLayout->addWidget(b, m_cnt/3, m_cnt%3);
    ++m_cnt;
    return b;
}


FormattingButton::FormattingButton( QWidget *parent)
    : QToolButton(parent)
    , m_lastId(0)
    , m_styleAction(0)
{
    m_menu = new QMenu();
    setPopupMode(MenuButtonPopup);
    setMenu(m_menu);
    if(m_styleAction==0)
        m_styleAction = new ItemChooserAction();
    m_menu->addAction(m_styleAction);
    connect(this, SIGNAL(released()), this, SLOT(itemSelected()));
}

void FormattingButton::addItem(QPixmap pm, int id)
{
    QToolButton *b = m_styleAction->addItem(pm);
    m_styleMap[b] = id;
    connect(b, SIGNAL(released()), this, SLOT(itemSelected()));

    if (!m_lastId) {
        m_lastId = id;
    }
}

void FormattingButton::addAction(QAction *action)
{
    m_menu->addAction(action);
}

void FormattingButton::addSeparator()
{
    m_menu->addSeparator();
}

void FormattingButton::itemSelected()
{
    if(sender() != this) {
        m_lastId = m_styleMap[sender()];
    }
    m_menu->hide();
    emit itemTriggered(m_lastId);
}

QString FormattingButton::example(KoListStyle::Style type) const {
    int value=1;
    switch( type ) {
        case KoListStyle::DecimalItem:
            return QString::number(value);
        case KoListStyle::AlphaLowerItem:
            return Lists::intToAlpha(value, Lists::Lowercase, false);
        case KoListStyle::UpperAlphaItem:
            return Lists::intToAlpha(value, Lists::Uppercase, false);
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

#include <FormattingButton.moc>
