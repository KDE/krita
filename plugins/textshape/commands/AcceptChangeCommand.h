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

#ifndef ACCEPTCHANGECOMMAND_H
#define ACCEPTCHANGECOMMAND_H

#include <KoTextCommandBase.h>

#include <QPair>

class KoChangeTracker;

class QTextDocument;

class AcceptChangeCommand : public QObject, public KoTextCommandBase
{
    Q_OBJECT
public:
    AcceptChangeCommand(int changeId, const QList<QPair<int, int> > &changeRanges, QTextDocument *document, KUndo2Command *parent = 0);
    ~AcceptChangeCommand();

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
};

#endif // ACCEPTCHANGECOMMAND_H
