/* This file is part of the KDE project
 * Copyright (C) 2008 Thorsten Zachmann <zachmann@kde.org>
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

#include "KoFindStrategy.h"

#include <KFind>
#include <KFindDialog>
#include <KMessageBox>
#include <KLocale>

#include "FindDirection_p.h"

class NonClosingFindDialog : public KFindDialog
{
public:
    NonClosingFindDialog(QWidget *parent)
            : KFindDialog(parent) {}

    virtual void accept() {}
};

KoFindStrategy::KoFindStrategy(QWidget * parent)
        : m_dialog(new NonClosingFindDialog(parent))
        , m_matches(0)
{
    m_dialog->setOptions(KFind::FromCursor);
}

KoFindStrategy::~KoFindStrategy()
{
    if(m_dialog->parent() == 0)
        delete m_dialog;
}

KFindDialog * KoFindStrategy::dialog()
{
    return m_dialog;
}

void KoFindStrategy::reset()
{
    m_matches = 0;
}

void KoFindStrategy::displayFinalDialog()
{
    KMessageBox::information(m_dialog, m_matches ? i18np("Found 1 match", "Found %1 matches", m_matches) : i18n("Found no match"));
    reset();
}

bool KoFindStrategy::foundMatch(QTextCursor & cursor, FindDirection * findDirection)
{
    ++m_matches;
    findDirection->select(cursor);
    return false;
}
