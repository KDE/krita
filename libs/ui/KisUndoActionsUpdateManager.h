/*
 *  SPDX-FileCopyrightText: 2018 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
