/*
 * Copyright (c) 2019 boud <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef TESTRESOURCESERVER_H
#define TESTRESOURCESERVER_H

#include <QObject>

#include "KisResourceTypes.h"
class KisResourceLocator;


class TestResourceServer : public QObject
{
    Q_OBJECT
private  Q_SLOTS:
    void initTestCase();
    void testFirstResource();
    void testResourceModel();
    void testResourceCount();
    void testRemoveResourceFromServer();
    void testSaveLocation();
    void testImportResourceFile();
    void testRemoveResourceFile();
    void testAddObserver();
    void testRemoveObserver();
    void testResourceByFileName();
    void testResourceByName();
    void testResourceByMD5();
    void testUpdateResource();
    void testAssignedTagsList();
    void cleanupTestCase();

private:

    QString m_srcLocation;
    QString m_dstLocation;

    KisResourceLocator *m_locator;
    const QString m_resourceType = ResourceType::PaintOpPresets;
};

#endif // TESTRESOURCESERVER_H
