/*
 * SPDX-FileCopyrightText: 2018 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef TESTTAGMODEL_H
#define TESTTAGMODEL_H

#include <QObject>
#include <QtSql>

#include "KisResourceTypes.h"
#include "KisTag.h"

class KisResourceLocator;
class TestTagModel : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();

    void testWithTagModelTester();

    void testRowCount();
    void testData();
    void testIndexForTag();
    void testTagForIndex();
    void testTagForUrl();
    void testAddEmptyTag();
    void testAddTag();
    void testSetTagActiveInactive();
    void testRenameTag();
    void testChangeTagActive();

    void testAddEmptyTagWithResources();
    void testAddTagWithResources();


    void cleanupTestCase();
private:

    QString m_srcLocation;
    QString m_dstLocation;

    KisTagSP m_tag;

    KisResourceLocator *m_locator;
    const QString m_resourceType = ResourceType::PaintOpPresets;

};

#endif
