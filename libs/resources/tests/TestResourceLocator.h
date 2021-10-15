/*
 * SPDX-FileCopyrightText: 2017 Boudewijn Rempt <boud@valdyas.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef TESTRESOURCELOCATOR_H
#define TESTRESOURCELOCATOR_H

#include <QObject>

class KisResourceLocator;


class TestResourceLocator : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void testLocatorInitialization();
    void testStorageInitialization();
    void testLocatorSynchronization();

    void testResourceLocationBase();
    void testResource();
    void testResourceForId();
    void testDocumentStorage();

    void cleanupTestCase();

    void testSyncVersions();

    void testImportExportResource();

private:

    QString m_srcLocation;
    QString m_dstLocation;

    KisResourceLocator *m_locator;
};

#endif
