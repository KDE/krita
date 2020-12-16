/*
 * SPDX-FileCopyrightText: 2018 boud <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef TESTRESOURCETYPEMODEL_H
#define TESTRESOURCETYPEMODEL_H

#include <QObject>
class KisResourceLocator;

class TestStorageModel : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void testRowCount();
    void testSetActive();
    void cleanupTestCase();
    void testMetaData();

private:

    QString m_srcLocation;
    QString m_dstLocation;
    KisResourceLocator *m_locator;
};

#endif
