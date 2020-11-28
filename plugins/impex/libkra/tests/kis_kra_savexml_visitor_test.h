/*
 *  SPDX-FileCopyrightText: 2007 Boudewijn Rempt boud @valdyas.org
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_KRA_SAVEXML_VISITOR_TEST_H
#define KIS_KRA_SAVEXML_VISITOR_TEST_H

#include <QtTest>

class KisKraSaveXmlVisitorTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();

    // XXX: Also test roundtripping of metadata
    void testCreateDomDocument();

};

#endif
