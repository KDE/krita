/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2006 Brad Hards <bradh@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KODOMTEST_H
#define KODOMTEST_H

#include <QByteArray>
#include <QObject>
#include "KoXmlReader.h"

class KoDomTest: public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void testQDom();
    void testKoDom();
private:
    KoXmlDocument m_doc;
};

#endif

