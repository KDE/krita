/*
 * SPDX-FileCopyrightText: 2020 boud <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef TESTAGFILTERTRESOURCEPROXYMODEL_H
#define TESTAGFILTERTRESOURCEPROXYMODEL_H

#include <QObject>
#include <QtSql>
#include "KisResourceTypes.h"

class KisResourceLocator;

class TestTagResourceModel : public QObject
{
    Q_OBJECT
private Q_SLOTS:

    void initTestCase();
    void testRowCount();
    void testData();
    void testTagResource();
    void testUntagResource();

    void testFilterTagResource();

    void cleanupTestCase();

private:

    QString m_srcLocation;
    QString m_dstLocation;

    KisResourceLocator *m_locator;
    const QString resourceType = ResourceType::PaintOpPresets;

};

#endif
