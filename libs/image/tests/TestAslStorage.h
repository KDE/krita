/*
 * SPDX-FileCopyrightText: 2018 boud <boud@valdyas.org>
 * SPDX-FileCopyrightText: 2019 Agata Cacko <cacko.azh@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef TESTASLSTORAGE_H
#define TESTASLSTORAGE_H


#include <QObject>

class TestAslStorage : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void testResourceIterator_data();
    void testResourceIterator();
    void testTagIterator_data();
    void testTagIterator();
    void testResourceItem_data();
    void testResourceItem();
    void testResource_data();
    void testResource();
};

#endif // TESTASLSTORAGE_H
