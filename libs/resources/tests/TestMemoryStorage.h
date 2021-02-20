/*
 * SPDX-FileCopyrightText: 2017 Boudewijn Rempt <boud@valdyas.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef TESTMEMORYSTORAGE_H
#define TESTMEMORYSTORAGE_H

#include <QObject>

class TestMemoryStorage : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testStorage();
    void testStorageRetrieval();
    void testTagIterator();
    void testAddResource();
    void initTestCase();
private:
};

#endif
