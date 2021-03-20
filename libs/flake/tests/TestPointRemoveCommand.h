/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2007 Thorsten Zachmann <zachmann@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/
#ifndef TESTPOINTREMOVECOMMAND_H
#define TESTPOINTREMOVECOMMAND_H

#include <QObject>

class TestPointRemoveCommand : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void redoUndoPointRemove();
    void redoUndoSubpathRemove();
    void redoUndoShapeRemove();
    void redoUndo();
};

#endif // TESTPOINTREMOVECOMMAND_H
