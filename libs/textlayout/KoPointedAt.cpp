/* This file is part of the KDE project
 * Copyright (C) 2011 C. Boemann, KO GmbH <cbo@kogmbh.com>
 * Copyright (C) 2011 C. Boemann <cbo@boemann.dk>
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

#include "KoPointedAt.h"

#include <KoInlineNote.h>
#include <KoInlineTextObjectManager.h>
#include <KoTextRangeManager.h>

#include <TextLayoutDebug.h>

#include <QTextCursor>

KoPointedAt::KoPointedAt()
    : position(-1)
    , bookmark(0)
    , note(0)
    , noteReference(-1)
    , table(0)
    , tableHit(None)
{
}

KoPointedAt::KoPointedAt(KoPointedAt *other)
{
    position = other->position;
    bookmark = other->bookmark;
    note = other->note;
    noteReference = other->noteReference;
    externalHRef = other->externalHRef;
    tableHit = other->tableHit;
    tableRowDivider = other->tableRowDivider;
    tableColumnDivider = other->tableColumnDivider;
    tableLeadSize = other->tableLeadSize;
    tableTrailSize = other->tableTrailSize;
    table = other->table;
}

void KoPointedAt::fillInLinks(const QTextCursor &cursor, KoInlineTextObjectManager *inlineManager, KoTextRangeManager *rangeManager)
{
    bookmark = 0;
    externalHRef.clear();
    note = 0;

    if (!inlineManager)
        return;

    // Is there an href here ?
    if (cursor.charFormat().isAnchor()) {
        QString href = cursor.charFormat().anchorHref();
        // local href starts with #
        if (href.startsWith('#')) {
            // however bookmark does not contain it, so strip it
            href = href.right(href.size() - 1);

            if (!href.isEmpty()) {
                bookmark = rangeManager->bookmarkManager()->bookmark(href);
            }
            return;
        } else {
            // Nope, then it must be external;
            externalHRef = href;
        }
    } else {
        note = dynamic_cast<KoInlineNote*>(inlineManager->inlineTextObject(cursor));
    }
}
