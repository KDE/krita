/*
 * SPDX-FileCopyrightText: 2019 boud <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef TESTAGFILTERTRESOURCEPROXYMODEL_H
#define TESTAGFILTERTRESOURCEPROXYMODEL_H

#include <QObject>
#include <QtSql>

class KisResourceLocator;

class TestStorageFilterProxyModel : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void testWithTagModelTester();
    void testFilterByName();
    void testFilterByType();
    void testFilterByActive();
    void cleanupTestCase();
private:

    QString m_srcLocation;
    QString m_dstLocation;

    KisResourceLocator *m_locator;

};

#endif
