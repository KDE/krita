/* This file is part of the KDE project
 * Copyright (C) 2013 Elvis Stansvik <elvstone@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */
#include "TestStyleManager.h"

#include "KoTextDocument.h"
#include "styles/KoCharacterStyle.h"
#include "styles/KoListStyle.h"
#include "styles/KoParagraphStyle.h"
#include "styles/KoSectionStyle.h"
#include "styles/KoStyleManager.h"
#include "styles/KoTableCellStyle.h"
#include "styles/KoTableColumnStyle.h"
#include "styles/KoTableRowStyle.h"
#include "styles/KoTableStyle.h"

#include <QSignalSpy>
#include <QTextBlock>
#include <QTextCursor>
#include <QTextDocument>
#include <QVariant>

#include <QTest>

#include "TextDebug.h"

void TestStyleManager::initTestCase()
{
    // Needed to use them in QVariant (for QSignalSpy).
    qRegisterMetaType<KoCharacterStyle *>("KoCharacterStyle *");
    qRegisterMetaType<const KoCharacterStyle *>("const KoCharacterStyle *");
    qRegisterMetaType<KoParagraphStyle *>("KoParagraphStyle *");
    qRegisterMetaType<const KoParagraphStyle *>("const KoParagraphStyle *");
    qRegisterMetaType<KoListStyle *>("KoListStyle *");
    qRegisterMetaType<KoTableStyle *>("KoTableStyle *");
    qRegisterMetaType<KoTableColumnStyle *>("KoTableColumnStyle *");
    qRegisterMetaType<KoTableRowStyle *>("KoTableRowStyle *");
    qRegisterMetaType<KoTableCellStyle *>("KoTableCellStyle *");
    qRegisterMetaType<KoSectionStyle *>("KoSectionStyle *");
}

void TestStyleManager::init()
{
debugText << "init";
    // Each test case starts with a document containing
    // "foo\nbar" and an empty style manager.
    m_doc = new QTextDocument();
    m_koDoc = new KoTextDocument(m_doc);
    QTextCursor(m_doc).insertText("foo\nbar");
    m_styleManager = new KoStyleManager(0);
    m_koDoc->setStyleManager(m_styleManager);
debugText << "init done";
}

void TestStyleManager::testAddRemoveCharacterStyle()
{
    // Add character style.
    KoCharacterStyle characterStyle;
    characterStyle.setName("Test Character Style");
    QSignalSpy addSignalSpy(m_styleManager, SIGNAL(styleAdded(KoCharacterStyle*)));
    m_styleManager->beginEdit();
    m_styleManager->add(&characterStyle);
    m_styleManager->endEdit();
    QVERIFY(characterStyle.styleId() > 0);
    QVERIFY(!m_styleManager->usedCharacterStyles().contains(characterStyle.styleId()));
    QCOMPARE(m_styleManager->characterStyles().count(&characterStyle), 1);
    QCOMPARE(m_styleManager->characterStyle(characterStyle.styleId()), &characterStyle);
    QCOMPARE(m_styleManager->characterStyle("Test Character Style"), &characterStyle);
    QCOMPARE(addSignalSpy.count(), 1);
    QCOMPARE(addSignalSpy.at(0).at(0).value<KoCharacterStyle *>(), &characterStyle);

    // Remove character style.
    QSignalSpy removeSignalSpy(m_styleManager, SIGNAL(styleRemoved(KoCharacterStyle*)));
    m_styleManager->beginEdit();
    m_styleManager->remove(&characterStyle);
    m_styleManager->endEdit();
    QVERIFY(!m_styleManager->characterStyles().contains(&characterStyle));
    QVERIFY(!m_styleManager->characterStyle(characterStyle.styleId()));
    QVERIFY(!m_styleManager->characterStyle("Test Character Style"));
    QCOMPARE(removeSignalSpy.count(), 1);
    QCOMPARE(removeSignalSpy.at(0).at(0).value<KoCharacterStyle *>(), &characterStyle);
}

void TestStyleManager::testAddRemoveParagraphStyle()
{
    // Add paragraph style.
    KoParagraphStyle paragraphStyle;
    paragraphStyle.setName("Test Paragraph Style");
    QSignalSpy addSignalSpy(m_styleManager, SIGNAL(styleAdded(KoParagraphStyle*)));
    m_styleManager->beginEdit();
    m_styleManager->add(&paragraphStyle);
    m_styleManager->endEdit();
    QVERIFY(paragraphStyle.styleId() > 0);
    QVERIFY(!m_styleManager->usedParagraphStyles().contains(paragraphStyle.styleId()));
    QCOMPARE(m_styleManager->paragraphStyles().count(&paragraphStyle), 1);
    QCOMPARE(m_styleManager->paragraphStyle(paragraphStyle.styleId()), &paragraphStyle);
    QCOMPARE(m_styleManager->paragraphStyle("Test Paragraph Style"), &paragraphStyle);
    QCOMPARE(addSignalSpy.count(), 1);
    QCOMPARE(addSignalSpy.at(0).at(0).value<KoParagraphStyle *>(), &paragraphStyle);

    // Remove paragraph style.
    QSignalSpy removeSignalSpy(m_styleManager, SIGNAL(styleRemoved(KoParagraphStyle*)));
    m_styleManager->beginEdit();
    m_styleManager->remove(&paragraphStyle);
    m_styleManager->endEdit();
    QVERIFY(!m_styleManager->paragraphStyles().contains(&paragraphStyle));
    QVERIFY(!m_styleManager->paragraphStyle(paragraphStyle.styleId()));
    QVERIFY(!m_styleManager->paragraphStyle("Test Paragraph Style"));
    QCOMPARE(removeSignalSpy.count(), 1);
    QCOMPARE(removeSignalSpy.at(0).at(0).value<KoParagraphStyle *>(), &paragraphStyle);
}

void TestStyleManager::testAddRemoveListStyle()
{
    // Add list style.
    KoListStyle listStyle;
    listStyle.setName("Test List Style");
    QSignalSpy addSignalSpy(m_styleManager, SIGNAL(styleAdded(KoListStyle*)));
    m_styleManager->beginEdit();
    m_styleManager->add(&listStyle);
    m_styleManager->endEdit();
    QVERIFY(listStyle.styleId() > 0);
    QCOMPARE(m_styleManager->listStyles().count(&listStyle), 1);
    QCOMPARE(m_styleManager->listStyle(listStyle.styleId()), &listStyle);
    QCOMPARE(m_styleManager->listStyle("Test List Style"), &listStyle);
    QCOMPARE(addSignalSpy.count(), 1);
    QCOMPARE(addSignalSpy.at(0).at(0).value<KoListStyle *>(), &listStyle);

    // Remove list style.
    QSignalSpy removeSignalSpy(m_styleManager, SIGNAL(styleRemoved(KoListStyle*)));
    m_styleManager->beginEdit();
    m_styleManager->remove(&listStyle);
    m_styleManager->endEdit();
    QVERIFY(!m_styleManager->listStyles().contains(&listStyle));
    QVERIFY(!m_styleManager->listStyle(listStyle.styleId()));
    QVERIFY(!m_styleManager->listStyle("Test List Style"));
    QCOMPARE(removeSignalSpy.count(), 1);
    QCOMPARE(removeSignalSpy.at(0).at(0).value<KoListStyle *>(), &listStyle);
}

void TestStyleManager::testAddRemoveTableStyle()
{
    // Add table style.
    KoTableStyle tableStyle;
    tableStyle.setName("Test Table Style");
    QSignalSpy addSignalSpy(m_styleManager, SIGNAL(styleAdded(KoTableStyle*)));
    m_styleManager->beginEdit();
    m_styleManager->add(&tableStyle);
    m_styleManager->endEdit();
    QVERIFY(tableStyle.styleId() > 0);
    QCOMPARE(m_styleManager->tableStyles().count(&tableStyle), 1);
    QCOMPARE(m_styleManager->tableStyle(tableStyle.styleId()), &tableStyle);
    QCOMPARE(m_styleManager->tableStyle("Test Table Style"), &tableStyle);
    QCOMPARE(addSignalSpy.count(), 1);
    QCOMPARE(addSignalSpy.at(0).at(0).value<KoTableStyle *>(), &tableStyle);

    // Remove table style.
    QSignalSpy removeSignalSpy(m_styleManager, SIGNAL(styleRemoved(KoTableStyle*)));
    m_styleManager->beginEdit();
    m_styleManager->remove(&tableStyle);
    m_styleManager->endEdit();
    QVERIFY(!m_styleManager->tableStyles().contains(&tableStyle));
    QVERIFY(!m_styleManager->tableStyle(tableStyle.styleId()));
    QVERIFY(!m_styleManager->tableStyle("Test Table Style"));
    QCOMPARE(removeSignalSpy.count(), 1);
    QCOMPARE(removeSignalSpy.at(0).at(0).value<KoTableStyle *>(), &tableStyle);
}

void TestStyleManager::testAddRemoveTableColumnStyle()
{
    // Add table column style.
    KoTableColumnStyle tableColumnStyle;
    tableColumnStyle.setName("Test Table Column Style");
    QSignalSpy addSignalSpy(m_styleManager, SIGNAL(styleAdded(KoTableColumnStyle*)));
    m_styleManager->beginEdit();
    m_styleManager->add(&tableColumnStyle);
    m_styleManager->endEdit();
    QVERIFY(tableColumnStyle.styleId() > 0);
    QCOMPARE(m_styleManager->tableColumnStyles().count(&tableColumnStyle), 1);
    QCOMPARE(m_styleManager->tableColumnStyle(tableColumnStyle.styleId()), &tableColumnStyle);
    QCOMPARE(m_styleManager->tableColumnStyle("Test Table Column Style"), &tableColumnStyle);
    QCOMPARE(addSignalSpy.count(), 1);
    QCOMPARE(addSignalSpy.at(0).at(0).value<KoTableColumnStyle *>(), &tableColumnStyle);

    // Remove table column style.
    QSignalSpy removeSignalSpy(m_styleManager, SIGNAL(styleRemoved(KoTableColumnStyle*)));
    m_styleManager->beginEdit();
    m_styleManager->remove(&tableColumnStyle);
    m_styleManager->endEdit();
    QVERIFY(!m_styleManager->tableColumnStyles().contains(&tableColumnStyle));
    QVERIFY(!m_styleManager->tableColumnStyle(tableColumnStyle.styleId()));
    QVERIFY(!m_styleManager->tableColumnStyle("Test Table Column Style"));
    QCOMPARE(removeSignalSpy.count(), 1);
    QCOMPARE(removeSignalSpy.at(0).at(0).value<KoTableColumnStyle *>(), &tableColumnStyle);
}

void TestStyleManager::testAddRemoveTableRowStyle()
{
    // Add table row style.
    KoTableRowStyle tableRowStyle;
    tableRowStyle.setName("Test Table Row Style");
    QSignalSpy addSignalSpy(m_styleManager, SIGNAL(styleAdded(KoTableRowStyle*)));
    m_styleManager->beginEdit();
    m_styleManager->add(&tableRowStyle);
    m_styleManager->endEdit();
    QVERIFY(tableRowStyle.styleId() > 0);
    QCOMPARE(m_styleManager->tableRowStyles().count(&tableRowStyle), 1);
    QCOMPARE(m_styleManager->tableRowStyle(tableRowStyle.styleId()), &tableRowStyle);
    QCOMPARE(m_styleManager->tableRowStyle("Test Table Row Style"), &tableRowStyle);
    QCOMPARE(addSignalSpy.count(), 1);
    QCOMPARE(addSignalSpy.at(0).at(0).value<KoTableRowStyle *>(), &tableRowStyle);

    // Remove table row style.
    QSignalSpy removeSignalSpy(m_styleManager, SIGNAL(styleRemoved(KoTableRowStyle*)));
    m_styleManager->beginEdit();
    m_styleManager->remove(&tableRowStyle);
    m_styleManager->endEdit();
    QVERIFY(!m_styleManager->tableRowStyles().contains(&tableRowStyle));
    QVERIFY(!m_styleManager->tableRowStyle(tableRowStyle.styleId()));
    QVERIFY(!m_styleManager->tableRowStyle("Test Table Row Style"));
    QCOMPARE(removeSignalSpy.count(), 1);
    QCOMPARE(removeSignalSpy.at(0).at(0).value<KoTableRowStyle *>(), &tableRowStyle);
}

void TestStyleManager::testAddRemoveTableCellStyle()
{
    // Add table cell style.
    KoTableCellStyle tableCellStyle;
    tableCellStyle.setName("Test Table Cell Style");
    QSignalSpy addSignalSpy(m_styleManager, SIGNAL(styleAdded(KoTableCellStyle*)));
    m_styleManager->beginEdit();
    m_styleManager->add(&tableCellStyle);
    m_styleManager->endEdit();
    QVERIFY(tableCellStyle.styleId() > 0);
    QCOMPARE(m_styleManager->tableCellStyles().count(&tableCellStyle), 1);
    QCOMPARE(m_styleManager->tableCellStyle(tableCellStyle.styleId()), &tableCellStyle);
    QCOMPARE(m_styleManager->tableCellStyle("Test Table Cell Style"), &tableCellStyle);
    QCOMPARE(addSignalSpy.count(), 1);
    QCOMPARE(addSignalSpy.at(0).at(0).value<KoTableCellStyle *>(), &tableCellStyle);

    // Remove table cell style.
    QSignalSpy removeSignalSpy(m_styleManager, SIGNAL(styleRemoved(KoTableCellStyle*)));
    m_styleManager->beginEdit();
    m_styleManager->remove(&tableCellStyle);
    m_styleManager->endEdit();
    QVERIFY(!m_styleManager->tableCellStyles().contains(&tableCellStyle));
    QVERIFY(!m_styleManager->tableCellStyle(tableCellStyle.styleId()));
    QVERIFY(!m_styleManager->tableCellStyle("Test Table Cell Style"));
    QCOMPARE(removeSignalSpy.count(), 1);
    QCOMPARE(removeSignalSpy.at(0).at(0).value<KoTableCellStyle *>(), &tableCellStyle);
}

void TestStyleManager::testAddRemoveSectionStyle()
{
    // Add section style.
    KoSectionStyle sectionStyle;
    sectionStyle.setName("Test Section Style");
    QSignalSpy addSignalSpy(m_styleManager, SIGNAL(styleAdded(KoSectionStyle*)));
    m_styleManager->beginEdit();
    m_styleManager->add(&sectionStyle);
    m_styleManager->endEdit();
    QVERIFY(sectionStyle.styleId() > 0);
    QCOMPARE(m_styleManager->sectionStyles().count(&sectionStyle), 1);
    QCOMPARE(m_styleManager->sectionStyle(sectionStyle.styleId()), &sectionStyle);
    QCOMPARE(m_styleManager->sectionStyle("Test Section Style"), &sectionStyle);
    QCOMPARE(addSignalSpy.count(), 1);
    QCOMPARE(addSignalSpy.at(0).at(0).value<KoSectionStyle *>(), &sectionStyle);

    // Remove section style.
    QSignalSpy removeSignalSpy(m_styleManager, SIGNAL(styleRemoved(KoSectionStyle*)));
    m_styleManager->beginEdit();
    m_styleManager->remove(&sectionStyle);
    m_styleManager->endEdit();
    QVERIFY(!m_styleManager->sectionStyles().contains(&sectionStyle));
    QVERIFY(!m_styleManager->sectionStyle(sectionStyle.styleId()));
    QVERIFY(!m_styleManager->sectionStyle("Test Section Style"));
    QCOMPARE(removeSignalSpy.count(), 1);
    QCOMPARE(removeSignalSpy.at(0).at(0).value<KoSectionStyle *>(), &sectionStyle);
}

void TestStyleManager::testAddAppliedCharacterStyle()
{
    // Create style, apply it, then add it to the manager.
    KoCharacterStyle characterStyle;
    QTextBlock block = m_doc->begin();
    characterStyle.applyStyle(block);

    m_styleManager->beginEdit();
    m_styleManager->add(&characterStyle);
    m_styleManager->endEdit();

    // Check that style is marked as used.
    QVERIFY(m_styleManager->usedCharacterStyles().contains(characterStyle.styleId()));
}

void TestStyleManager::testApplyAddedCharacterStyle()
{
    QSignalSpy appliedSignalSpy(m_styleManager, SIGNAL(styleApplied(const KoCharacterStyle*)));

    // Create style, add it to the manager, then apply it.
    KoCharacterStyle characterStyle;

    m_styleManager->beginEdit();
    m_styleManager->add(&characterStyle);
    m_styleManager->endEdit();

    QTextBlock block = m_doc->begin();
    characterStyle.applyStyle(block);

    // Check that style is marked as used and that the correct signal was emitted.
    QVERIFY(m_styleManager->usedCharacterStyles().contains(characterStyle.styleId()));
    QCOMPARE(appliedSignalSpy.count(), 1);
    QCOMPARE(appliedSignalSpy.at(0).at(0).value<const KoCharacterStyle *>(), &characterStyle);
}

void TestStyleManager::testAddAppliedParagraphStyle()
{
    // Create style, apply it, then add it to the manager.
    KoParagraphStyle paragraphStyle;
    QTextBlock block = m_doc->begin();
    paragraphStyle.applyStyle(block);

    m_styleManager->beginEdit();
    m_styleManager->add(&paragraphStyle);
    m_styleManager->endEdit();

    // Check that style is marked as used.
    QVERIFY(m_styleManager->usedParagraphStyles().contains(paragraphStyle.styleId()));
}

void TestStyleManager::testApplyAddedParagraphStyle()
{
    QSignalSpy appliedSignalSpy(m_styleManager, SIGNAL(styleApplied(const KoParagraphStyle*)));

    // Create style, add it to the manager, then apply it.
    KoParagraphStyle paragraphStyle;

    m_styleManager->beginEdit();
    m_styleManager->add(&paragraphStyle);
    m_styleManager->endEdit();

    QTextBlock block = m_doc->begin();
    paragraphStyle.applyStyle(block);

    // Check that style is marked as used and that the correct signal was emitted.
    QVERIFY(m_styleManager->usedParagraphStyles().contains(paragraphStyle.styleId()));
    QCOMPARE(appliedSignalSpy.count(), 1);
    QCOMPARE(appliedSignalSpy.at(0).at(0).value<const KoParagraphStyle *>(), &paragraphStyle);
}

void TestStyleManager::cleanup()
{
    delete m_doc;
    delete m_koDoc;
    delete m_styleManager;
}

QTEST_MAIN(TestStyleManager)
