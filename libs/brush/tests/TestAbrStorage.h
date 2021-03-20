/*
 * SPDX-FileCopyrightText: 2018 boud <boud@valdyas.org>
 * SPDX-FileCopyrightText: 2019 Agata Cacko <cacko.azh@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef TESTABRSTORAGE_H
#define TESTABRSTORAGE_H


#include <QObject>

class TestAbrStorage : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void testResourceIterator();
    void testTagIterator();
    void testResourceItem();
    void testResource();
};

#endif // TESTABRSTORAGE_H
