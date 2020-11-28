/*
 * SPDX-FileCopyrightText: 2016 Boudewijn Rempt <boud@valdyas.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef DLGCONFIGUREHISTORYDOCK_H
#define DLGCONFIGUREHISTORYDOCK_H

#include <KoDialog.h>
#include <QObject>
#include <QWidget>
#include <kundo2stack.h>
#include <KisUndoView.h>

class DlgConfigureHistoryDock : public KoDialog
{
    Q_OBJECT
public:
    DlgConfigureHistoryDock(KisUndoView *view, KUndo2QStack *stack, QWidget *parent = 0);
private:
    KUndo2QStack *m_stack;
};

#endif // DLGCONFIGUREHISTORYDOCK_H
