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



#include "KoFindBase.h"

class KoFindBase::Private
{
public:
    Private() : currentMatch(0) { }
    KoFindMatchList matches;
    int currentMatch;
};

KoFindBase::KoFindBase(QObject* parent)
    : QObject(parent), d(new Private)
{
}

KoFindBase::~KoFindBase()
{
}

const KoFindBase::KoFindMatchList& KoFindBase::matches() const
{
    return d->matches;
}

bool KoFindBase::hasMatches() const
{
    return d->matches.count() > 0;
}

KoFindMatch KoFindBase::currentMatch() const
{
    if(d->matches.count() > 0 && d->currentMatch < d->matches.count())
    {
        return d->matches.at(d->currentMatch);
    }
    return KoFindMatch();
}

void KoFindBase::setMatches(const KoFindBase::KoFindMatchList& matches)
{
    d->matches = matches;
}

void KoFindBase::setCurrentMatch(int index)
{
    d->currentMatch = index;
}

void KoFindBase::find(const QString& pattern)
{
    clearMatches();
    d->matches.clear();
    findImpl(pattern, d->matches);

    emit hasMatchesChanged(d->matches.count() > 0);
    if(d->matches.count() > 0) {
        emit highlight();
        emit matchFound(d->matches.at(0));
    } else {
        emit noMatchFound();
    }
}

void KoFindBase::findNext()
{
    if(d->matches.count() == 0) {
        return;
    }

    d->currentMatch++;
    if(d->currentMatch >= d->matches.count()) {
        d->currentMatch = 0;
        emit wrapAround();
    }
    emit matchFound(d->matches.at(d->currentMatch));
}

void KoFindBase::findPrevious()
{
    if(d->matches.count() == 0) {
        return;
    }
    
    d->currentMatch--;
    if(d->currentMatch < 0) {
        d->currentMatch = d->matches.count() - 1;
        emit wrapAround();
    }
    emit matchFound(d->matches.at(d->currentMatch));
}

void KoFindBase::finished()
{
    clearMatches();
    d->matches.clear();
}

void KoFindBase::clearMatches()
{

}
