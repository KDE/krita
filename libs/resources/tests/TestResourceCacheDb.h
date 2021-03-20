/*
 * SPDX-FileCopyrightText: 2017 Boudewijn Rempt <boud@valdyas.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef TESTRESOURCECACHEDB_H
#define TESTRESOURCECACHEDB_H

#include <QObject>

class TestResourceCacheDb : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void testCreateDatabase();
    void testLookupTables();
    void testMetaData();
    void cleanupTestCase();
private:
};

#endif
