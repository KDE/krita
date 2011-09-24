/* This file is part of the KDE project
 * Copyright (C) 2011 Gopalakrishna Bhat A <gopalakbhat@gmail.com>
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

#ifndef KOTEXTLAYOUTSCHEDULER_H
#define KOTEXTLAYOUTSCHEDULER_H

#include <QAbstractTextDocumentLayout>

/**
 * This class is used to indicate to the layout that a document has changed and it needs a relayout
 * Use the markDocumentChanged method to mark a document dirty
 */
class KoTextLayoutScheduler
{
public:
    /// Marks that a document dirty(changed) indicating to the layout library that this needs a re-layout
    static void markDocumentChanged(const QTextDocument *document, int position, int charsRemoved, int charsAdded);

    class KoTextLayoutSchedulerInternal: public QAbstractTextDocumentLayout
    {
    public:
        void markDocumentDirty(int from, int charsRemoved, int charsAdded);
    };
};

#endif // KOTEXTLAYOUTSCHEDULER_H
