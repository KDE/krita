/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2017 Boudewijn Rempt

   SPDX-License-Identifier: LGPL-2.0-or-later
*/
#ifndef TESTFILTER_H
#define TESTFILTER_H

#include <QObject>

class TestFilter : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testApply();
    void testStartFilter();
};

#endif

