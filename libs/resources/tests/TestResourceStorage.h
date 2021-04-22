/*
 * SPDX-FileCopyrightText: 2017 Boudewijn Rempt <boud@valdyas.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef TESTRESOURCESTORAGE_H
#define TESTRESOURCESTORAGE_H

#include <QObject>

class KisResourceLocator;

class TestResourceStorage : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void testStorage();
    void testImportResourceFile();
    void cleanupTestCase();
private:
    QString m_dstLocation;
    KisResourceLocator *m_locator;

};

#endif
