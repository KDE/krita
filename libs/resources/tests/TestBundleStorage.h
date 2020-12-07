/*
 * Copyright (c) 2018 boud <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef TESTBUNDLESTORAGE_H
#define TESTBUNDLESTORAGE_H

#include <QObject>

class TestBundleStorage : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void testMetaData();
    void testResourceIterator();
    void testTagIterator();
    void testResourceItem();
    void testResource();
};

#endif // TESTBUNDLESTORAGE_H
