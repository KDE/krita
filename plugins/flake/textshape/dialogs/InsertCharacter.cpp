/* This file is part of the KDE project
 * Copyright (C) 2007 Thomas Zander <zander@kde.org>
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
#include "InsertCharacter.h"

#include <klocalizedstring.h>
#include <kcharselect.h>

#include <QMainWindow>
#include <QGridLayout>
#include <QPushButton>

InsertCharacter::InsertCharacter(QWidget *parent)
    : QDockWidget(i18n("Special Characters"))
{
    QWidget *specialCharacterWidget = new QWidget();
    QGridLayout *lay = new QGridLayout(specialCharacterWidget);
    lay->setMargin(6);
    m_charSelector = new KCharSelect(specialCharacterWidget,
                                     0,
                                     KCharSelect::SearchLine | KCharSelect::FontCombo | KCharSelect::BlockCombos |
                                     KCharSelect::CharacterTable | KCharSelect::DetailBrowser);
    lay->addWidget(m_charSelector, 0, 0, 1, 3);
    QPushButton *insert = new QPushButton(i18n("Insert"), specialCharacterWidget);
    lay->addWidget(insert, 1, 1);
    QPushButton *close = new QPushButton(i18nc("Close dialog", "Close"), specialCharacterWidget);
    lay->addWidget(close, 1, 2);
    lay->setColumnStretch(0, 9);

    setObjectName("insertSpecialCharacter");
    setWidget(specialCharacterWidget);
    while (parent->parentWidget()) {
        parent = parent->parentWidget();
    }
    QMainWindow *mw = dynamic_cast<QMainWindow *>(parent);
    if (mw) {
        mw->addDockWidget(Qt::TopDockWidgetArea, this);
    }
    setFloating(true);

    connect(close, SIGNAL(released()), this, SLOT(hide()));
    connect(insert, SIGNAL(released()), this, SLOT(insertCharacter()));
    connect(m_charSelector, SIGNAL(charSelected(QChar)), this, SLOT(insertCharacter()));
}

void InsertCharacter::insertCharacter()
{
    emit insertCharacter(QString(m_charSelector->currentChar()));
}
