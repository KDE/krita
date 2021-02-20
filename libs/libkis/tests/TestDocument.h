/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2017 Boudewijn Rempt

   SPDX-License-Identifier: LGPL-2.0-or-later
*/
#ifndef TESTDOCUMENT_H
#define TESTDOCUMENT_H

#include <QObject>

class TestDocument : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testSetColorSpace();
    void testSetColorProfile();
    void testPixelData();
    void testThumbnail();
    void testCreateFillLayer();
    void testAnnotations();
};

#endif

