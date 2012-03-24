/* This file is part of the KDE project
 * Copyright (C) 2011 Casper Boemann, KO GmbH <cbo@kogmbh.com>
 * Copyright (C) 2011 Casper Boemann <cbo@boemann.dk>
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

#include <KoBookmark.h>
#include <KoInlineTextObjectManager.h>

#include <KDebug>

#include <QTextCursor>

KoPointedAt::KoPointedAt()
    : position(-1)
    , bookmark(0)
    , table(0)
    , tableHit(None)
{
}

KoPointedAt::KoPointedAt(KoPointedAt *other)
{
    position = other->position;
    bookmark = other->bookmark;
    externalHRef = other->externalHRef;
    tableHit = other->tableHit;
    tableRowDivider = other->tableRowDivider;
    tableColumnDivider = other->tableColumnDivider;
    tableLeadSize = other->tableLeadSize;
    tableTrailSize = other->tableTrailSize;
    table = other->table;
}

void KoPointedAt::fillInBookmark(QTextCursor cursor, KoInlineTextObjectManager *inlineManager)
{
    bookmark = 0;
    externalHRef.clear();

    if (!inlineManager)
        return;

    cursor.setPosition(position);

    // Is there an href here ?
    if (cursor.charFormat().isAnchor()) {
        QString href = cursor.charFormat().anchorHref();
        // local href starts with #
        if (href.startsWith("#")) {
            // however bookmark does not contain it, so strip it
            href = href.right(href.size() - 1);

            if (!href.isEmpty()) {
                bookmark = inlineManager->bookmarkManager()->retrieveBookmark(href);
            }
            return;
        } else {
            // Nope, then it must be external;
            externalHRef = href;
        }
    }
}
