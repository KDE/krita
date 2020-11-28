/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2017 Boudewijn Rempt

   SPDX-License-Identifier: LGPL-2.0-or-later
*/
#ifndef TESTNODE_H
#define TESTNODE_H

#include <QObject>

class TestNode : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testSetColorSpace();
    void testSetColorProfile();
    void testPixelData();
    void testProjectionPixelData();
    void testThumbnail();
    void testMergeDown();
};

#endif

