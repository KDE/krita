/* This file is part of the KDE project
* Copyright (C) 2010 Pierre Stirnweiss \pstirnweiss@googlemail.com>
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

#ifndef REJECTCHANGECOMMAND_H
#define REJECTCHANGECOMMAND_H

#include <KoTextCommandBase.h>

#include <QPair>

class KoChangeTracker;
class KoTextDocumentLayout;

class QTextDocument;

class RejectChangeCommand : public QObject, public KoTextCommandBase
{
    Q_OBJECT
public:
    RejectChangeCommand(int changeId, const QList<QPair<int, int> > &changeRanges, QTextDocument *document, KUndo2Command *parent = 0);
    ~RejectChangeCommand();

    virtual void redo();
    virtual void undo();

Q_SIGNALS:
    void acceptRejectChange();

private:
    bool m_first;
    int m_changeId;
    QList<QPair<int, int> > m_changeRanges;
    QTextDocument *m_document;
    KoChangeTracker *m_changeTracker;
    KoTextDocumentLayout *m_layout;
};

#endif // REJECTCHANGECOMMAND_H
