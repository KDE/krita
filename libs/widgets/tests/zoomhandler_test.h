/*
 *  SPDX-FileCopyrightText: 2007 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef ZOOMHANDLER_TEST_H
#define ZOOMHANDLER_TEST_H

#include <QObject>

class zoomhandler_test : public QObject
{
    Q_OBJECT

private Q_SLOTS:

    // tests
    void testConstruction();
    void testApi();
    void testViewToDocument();
    void testDocumentToView();

};

#endif
