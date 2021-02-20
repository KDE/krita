/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2017 Boudewijn Rempt

   SPDX-License-Identifier: LGPL-2.0-or-later
*/
#ifndef TESTCHANNEL_H
#define TESTCHANNEL_H

#include <QObject>

class TestChannel : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testPixelDataU8();
    void testPixelDataU16();
    void testPixelDataF16();
    void testPixelDataF32();
    void testReadWritePixelData();
};

#endif

