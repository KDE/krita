/*
 *  Copyright (c) 2018 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef KISUNDOACTIONSUPDATEMANAGER_H
#define KISUNDOACTIONSUPDATEMANAGER_H

#include <QObject>
#include <kis_signal_auto_connection.h>

class QAction;
class KisDocument;

class KisUndoActionsUpdateManager : public QObject
{
    Q_OBJECT
public:
    KisUndoActionsUpdateManager(QAction *undoAction, QAction *redoAction, QObject *parent = 0);

    void setCurrentDocument(KisDocument *document);

public Q_SLOTS:
    void slotUndoTextChanged(const QString &text);
    void slotRedoTextChanged(const QString &text);

private:
    QAction *m_undoAction;
    QAction *m_redoAction;

    KisSignalAutoConnectionsStore m_documentConnections;
};

#endif // KISUNDOACTIONSUPDATEMANAGER_H
