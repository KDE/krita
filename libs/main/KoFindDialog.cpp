/* This file is part of the KDE project
 *
 * Copyright (c) 2010 Arjen Hiemstra <ahiemstra@heimr.nl>
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

#include "KoFindDialog.h"

#include <QtGui/QApplication>
#include <KDE/KFindDialog>
#include <KDE/KFind>
#include <KDE/KAction>
#include <KDE/KActionCollection>

#include "KoFindBase.h"

class KoFindDialog::Private
{
public:
    KoFindBase* find;
};

KoFindDialog::KoFindDialog(KoFindBase* find, KActionCollection* ac, QWidget* parent)
    : KFindDialog(parent), d(new Private)
{
    d->find = find;

    //connect(provider, SIGNAL(resourceChanged(int, const QVariant&)), this, SLOT(resourceChanged(int, const QVariant&)));
    ac->addAction(KStandardAction::Find, "edit_find", this, SLOT(show()));

    KAction *findNextAction = ac->addAction(KStandardAction::FindNext, "edit_findnext", find, SLOT(findNext()));
    connect(find, SIGNAL(hasMatchesChanged(bool)), findNextAction, SLOT(setEnabled(bool)));
    findNextAction->setEnabled(false);
    KAction *findPrevAction = ac->addAction(KStandardAction::FindPrev, "edit_findprevious", find, SLOT(findPrevious()));
    connect(find, SIGNAL(hasMatchesChanged(bool)), findPrevAction, SLOT(setEnabled(bool)));
    findPrevAction->setEnabled(false);

    //ac->addAction(KStandardAction::Replace, "edit_replace", this, SLOT(replaceActivated()));
}

KoFindDialog::~KoFindDialog()
{
    delete d;
}

void KoFindDialog::accept()
{
    if(d->find->hasMatches()) {
        if( options() & KFind::FindBackwards ) {
            d->find->findPrevious();
        } else {
            d->find->findNext();
        }
    } else {
        d->find->find( pattern() );
    }
}

