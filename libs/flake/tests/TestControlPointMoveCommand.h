/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2007 Thorsten Zachmann <zachmann@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/
#ifndef TESTCONTROLPOINTMOVECOMMAND_H
#define TESTCONTROLPOINTMOVECOMMAND_H

#include <QObject>

class TestControlPointMoveCommand : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void redoUndoControlPoint1();
    void redoUndoControlPoint1Smooth();
    void redoUndoControlPoint1Symmetric();
    void redoUndoControlPoint2();
    void redoUndoControlPoint2Smooth();
    void redoUndoControlPoint2Symmetric();
};

#endif // TESTCONTROLPOINTMOVECOMMAND_H
