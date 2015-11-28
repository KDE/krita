/*
 This file is part of the KDE project
 * Copyright (C) 2009 Ganesh Paramasivam <ganesh@crystalfab.com>
 * Copyright (C) 2009 Pierre Stirnweiss <pstirnweiss@googlemail.com>
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
 * Boston, MA 02110-1301, USA.*/

#ifndef SHOWCHANGECOMMAND_H
#define SHOWCHANGECOMMAND_H

#include "KoTextCommandBase.h"
#include <QObject>
#include <QList>

class KoChangeTracker;
class KoTextEditor;
class KoCanvasBase;

class QTextDocument;

class ShowChangesCommand : public QObject, public KoTextCommandBase
{
    Q_OBJECT
public:

    ShowChangesCommand(bool showChanges, QTextDocument *document, KoCanvasBase *canvas, KUndo2Command *parent = 0);
    ~ShowChangesCommand();

    virtual void undo();
    virtual void redo();

Q_SIGNALS:
    void toggledShowChange(bool on);

private:
    void enableDisableChanges();
    void enableDisableStates(bool showChanges);
    void insertDeletedChanges();
    void checkAndAddAnchoredShapes(int position, int length);
    void removeDeletedChanges();
    void checkAndRemoveAnchoredShapes(int position, int length);

    QTextDocument *m_document;
    KoChangeTracker *m_changeTracker;
    KoTextEditor *m_textEditor;
    bool m_first;
    bool m_showChanges;
    KoCanvasBase *m_canvas;

    QList<KUndo2Command *> m_shapeCommands;
};

#endif // SHOWCHANGECOMMAND_H
