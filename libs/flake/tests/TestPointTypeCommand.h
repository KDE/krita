/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2007 Thorsten Zachmann <zachmann@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/
#ifndef TESTPOINTTYPECOMMAND_H
#define TESTPOINTTYPECOMMAND_H

#include <QObject>

class TestPointTypeCommand : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void redoUndoSymmetric();
    void redoUndoSmooth();
    void redoUndoCorner();
};

#endif // TESTPOINTTYPECOMMAND_H
