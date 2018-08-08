/* This file is part of the KDE project
 * Copyright (C) 2006-2009 Thomas Zander <zander@kde.org>
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

#include "KoTextRange.h"

#include "KoTextRangeManager.h"
#include "KoTextInlineRdf.h"

#include "TextDebug.h"
#include <QTextCursor>

class KoTextRangePrivate
{
public:
    KoTextRangePrivate()
        : manager(0)
        , id(-1)
        , rdf(0)
        , positionOnlyMode(true)
    {
    }
    virtual ~KoTextRangePrivate();

    KoTextRangeManager *manager;
    int id;
    QTextCursor cursor;
    KoTextInlineRdf *rdf; //< A textrange might have RDF, we own it.
    bool positionOnlyMode;
    int snapAnchor;
    int snapPos;
};

KoTextRange::KoTextRange(const QTextCursor &cursor)
    : d_ptr(new KoTextRangePrivate)
{
    d_ptr->cursor = cursor;
    d_ptr->cursor.setPosition(cursor.selectionStart());
    d_ptr->cursor.setKeepPositionOnInsert(true);
    if (cursor.hasSelection()) {
        setRangeEnd(cursor.selectionEnd());
    }
}

KoTextRangePrivate::~KoTextRangePrivate()
{
    delete rdf;
}


KoTextRange::~KoTextRange()
{
    if (d_ptr->manager) {
        d_ptr->manager->remove(this);
    }
    delete d_ptr;
    d_ptr = 0;
}

void KoTextRange::setManager(KoTextRangeManager *manager)
{
    Q_D(KoTextRange);
    d->manager = manager;
}

KoTextRangeManager *KoTextRange::manager() const
{
    Q_D(const KoTextRange);
    return d->manager;
}

QTextDocument *KoTextRange::document() const
{
    Q_D(const KoTextRange);
    return d->cursor.document();
}

bool KoTextRange::positionOnlyMode() const
{
    Q_D(const KoTextRange);
    return d->positionOnlyMode;
}

void KoTextRange::setPositionOnlyMode(bool b)
{
    Q_D(KoTextRange);
    d->positionOnlyMode = b;
}

bool KoTextRange::hasRange() const
{
    Q_D(const KoTextRange);
    return (!d->positionOnlyMode) && d->cursor.hasSelection();
}

int KoTextRange::rangeStart() const
{
    Q_D(const KoTextRange);
    return d->positionOnlyMode ? d->cursor.position() : d->cursor.selectionStart();
}

int KoTextRange::rangeEnd() const
{
    Q_D(const KoTextRange);
    return d->positionOnlyMode ? d->cursor.position() : d->cursor.selectionEnd();
}

void KoTextRange::setRangeStart(int position)
{
    Q_D(KoTextRange);
    d->positionOnlyMode = true;
    d->cursor.setPosition(position);
}

void KoTextRange::setRangeEnd(int position)
{
    Q_D(KoTextRange);
    d->positionOnlyMode = false;
    d->cursor.setPosition(d->cursor.selectionStart());
    d->cursor.setPosition(position, QTextCursor::KeepAnchor);
}

QString KoTextRange::text() const
{
    Q_D(const KoTextRange);
    return d->positionOnlyMode ? QString() : d->cursor.selectedText();
}

void KoTextRange::setInlineRdf(KoTextInlineRdf* rdf)
{
    Q_D(KoTextRange);
    d->rdf = rdf;
}

KoTextInlineRdf* KoTextRange::inlineRdf() const
{
    Q_D(const KoTextRange);
    return d->rdf;
}

void KoTextRange::snapshot()
{
    Q_D(KoTextRange);
    d->snapAnchor = d->cursor.anchor();
    d->snapPos = d->cursor.position();
}

void KoTextRange::restore()
{
    Q_D(KoTextRange);
    d->cursor.setPosition(d->snapAnchor);
    d->cursor.setPosition(d->snapPos, QTextCursor::KeepAnchor);
}
