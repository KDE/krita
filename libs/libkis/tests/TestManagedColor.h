/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2017 Boudewijn Rempt

   SPDX-License-Identifier: LGPL-2.0-or-later
*/
#ifndef TESTMANAGEDCOLOR_H
#define TESTMANAGEDCOLOR_H

#include <QObject>

class TestManagedColor : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testOperatorIs();
    void testSetColorSpace();
    void testComponentsRoundTrip();
    void testXMLRoundTrip();
    void testToQString();
};

#endif

