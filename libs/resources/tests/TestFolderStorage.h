/*
 * SPDX-FileCopyrightText: 2017 Boudewijn Rempt <boud@valdyas.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef TESTFOLDERSTORAGE_H
#define TESTFOLDERSTORAGE_H

#include <QObject>

class KisResourceLocator;

class TestFolderStorage : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void testStorage();
    void testTagIterator();
    void testAddResource();
    void cleanupTestCase();
private:
    QString m_srcLocation;
    QString m_dstLocation;
    KisResourceLocator *m_locator;

};

#endif
