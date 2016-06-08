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
#ifndef KOPOINTEDAT_H
#define KOPOINTEDAT_H

#include "kritatextlayout_export.h"

#include <QString>
#include <QTextCursor>
#include <QPointF>

class KoBookmark;
class QTextTable;
class KoInlineTextObjectManager;
class KoTextRangeManager;
class KoInlineNote;

class KRITATEXTLAYOUT_EXPORT KoPointedAt
{
public:
    KoPointedAt();
    explicit KoPointedAt(KoPointedAt *other);

    void fillInLinks(const QTextCursor &cursor, KoInlineTextObjectManager *inlineManager, KoTextRangeManager *rangeManager);

    enum TableHit {
          None
        , ColumnDivider
        , RowDivider
    };
    int position;
    KoBookmark *bookmark;
    QString externalHRef;
    KoInlineNote *note;
    int noteReference;
    QTextTable *table;
    TableHit tableHit;
    int tableRowDivider;
    int tableColumnDivider;
    qreal tableLeadSize;
    qreal tableTrailSize;
    QPointF tableDividerPos;

};

#endif
