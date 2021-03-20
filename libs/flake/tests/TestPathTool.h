/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2007 Thorsten Zachmann <zachmann@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/
#ifndef TESTPATHTOOL_H
#define TESTPATHTOOL_H

#include <QObject>

class TestPathTool : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void koPathPointSelection_selectedSegmentsData();
};

#endif // TESTPATHTOOL_H
