/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2007 Thorsten Zachmann <zachmann@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/
#ifndef TESTREMOVESUBPATHCOMMAND_H
#define TESTREMOVESUBPATHCOMMAND_H

#include <QObject>

class TestRemoveSubpathCommand : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void redoUndo();
};

#endif // TESTREMOVESUBPATHCOMMAND_H
