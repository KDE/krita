/* This file is part of the KDE project
 * Copyright (C) 2007 Thomas Zander <zander@kde.org>
 * Copyright (C) 2008 Fredy Yanardi <fyanardi@gmail.com>
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

#include "KoFind.h"

#include <KoResourceManager.h>
#include <KActionCollection>
#include <KAction>

#include "KoFind_p.h"

KoFind::KoFind(QWidget *parent, KoResourceManager *provider, KActionCollection *ac)
        : QObject(parent)
        , d(new KoFindPrivate(this, provider, parent))
{
    connect(provider, SIGNAL(resourceChanged(int, const QVariant&)), this, SLOT(resourceChanged(int, const QVariant&)));
    ac->addAction(KStandardAction::Find, "edit_find", this, SLOT(findActivated()));
    d->findNext = ac->addAction(KStandardAction::FindNext, "edit_findnext", this, SLOT(findNextActivated()));
    d->findNext->setEnabled(false);
    d->findPrev = ac->addAction(KStandardAction::FindPrev, "edit_findprevious", this, SLOT(findPreviousActivated()));
    d->findPrev->setEnabled(false);
    ac->addAction(KStandardAction::Replace, "edit_replace", this, SLOT(replaceActivated()));
}

KoFind::~KoFind()
{
    delete d;
}

#include <KoFind.moc>
