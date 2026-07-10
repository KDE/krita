/*
 *  SPDX-FileCopyrightText: 2026 Moritz Staudinger
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_view_signals_test.h"

#include <QApplication>
#include <QScopedPointer>
#include <QSignalSpy>
#include <QTimer>

#include <KisDocument.h>
#include <KisMainWindow.h>
#include <KisPart.h>
#include <KisView.h>
#include <KisViewManager.h>

#include <KoInteractionTool.h>
#include <KoToolManager.h>

#include <Canvas.h>
#include <Krita.h>
#include <ManagedColor.h>
#include <Resource.h>
#include <View.h>

#include <testui.h>
#include <util.h>

void KisViewSignalsTest::initTestCase()
{
    Q_INIT_RESOURCE(krita);

    m_document = createEmptyDocument();
    QVERIFY(m_document);

    m_mainWindow = KisPart::instance()->createMainWindow();
    QVERIFY(m_mainWindow);

    m_kisView = m_mainWindow->newView(m_document);
    QVERIFY(m_kisView);

    m_viewManager = m_mainWindow->viewManager();
    QVERIFY(m_viewManager);
    m_viewManager->setCurrentView(m_kisView);

    const auto timers = m_mainWindow->findChildren<QTimer*>();
    for (QTimer *timer : timers) {
        if (timer && timer->isSingleShot() && timer->interval() == 1000) {
            timer->stop();
        }
    }

    QApplication::processEvents();

    m_view = new View(m_kisView);
    QVERIFY(m_view);
}

void KisViewSignalsTest::cleanupTestCase()
{
    delete m_view;
    m_view = nullptr;

    if (m_kisView) {
        m_kisView->setViewManager(nullptr);
        m_kisView->closeView();
        QApplication::sendPostedEvents();
        QApplication::processEvents();
        m_kisView = nullptr;
    }

    m_viewManager = nullptr;

    if (m_mainWindow) {
        m_mainWindow->hide();
        QApplication::processEvents();
        delete m_mainWindow;
        m_mainWindow = nullptr;
        QApplication::sendPostedEvents();
        QApplication::processEvents();
    }

    delete m_document;
    m_document = nullptr;
    QApplication::sendPostedEvents();
    QApplication::processEvents();
}

void KisViewSignalsTest::testCurrentToolChanged()
{
    const QString brushToolId = "KritaShape/KisToolBrush";
    const QString currentToolId = KoToolManager::instance()->activeToolId();
    const QString targetToolId = currentToolId == brushToolId ? QString::fromLatin1(KoInteractionTool_ID) : brushToolId;

    QSignalSpy spy(m_view, SIGNAL(currentToolChanged(QString)));
    QVERIFY(spy.isValid());

    KoToolManager::instance()->switchToolRequested(targetToolId);
    QApplication::processEvents();

    QVERIFY(spy.count() >= 1);
    QCOMPARE(spy.takeFirst().at(0).toString(), targetToolId);
    QCOMPARE(KoToolManager::instance()->activeToolId(), targetToolId);
}

void KisViewSignalsTest::testCurrentBrushPresetChanged()
{
    QString type = "preset";
    QMap<QString, Resource*> presets = Krita().resources(type);
    if (presets.size() < 2) {
        qDeleteAll(presets);
        QSKIP("Need at least two paintop presets to test preset change signals.");
    }

    Resource *initialPreset = nullptr;
    Resource *targetPreset = nullptr;
    for (auto it = presets.begin(); it != presets.end(); ++it) {
        if (!it.value()) {
            continue;
        }

        if (!initialPreset) {
            initialPreset = it.value();
            continue;
        }

        if (!targetPreset) {
            targetPreset = it.value();
            break;
        }
    }

    if (!initialPreset || !targetPreset) {
        qDeleteAll(presets);
        QSKIP("Could not obtain two valid paintop presets from the resource model.");
    }

    m_view->setCurrentBrushPreset(initialPreset);
    QApplication::processEvents();

    QScopedPointer<Resource> currentPreset(m_view->currentBrushPreset());
    QVERIFY(currentPreset);
    QCOMPARE(currentPreset->name(), initialPreset->name());

    QSignalSpy spy(m_view, SIGNAL(currentBrushPresetChanged()));
    QVERIFY(spy.isValid());

    m_view->setCurrentBrushPreset(targetPreset);
    QApplication::processEvents();

    QVERIFY(spy.count() >= 1);

    QScopedPointer<Resource> changedPreset(m_view->currentBrushPreset());
    QVERIFY(changedPreset);
    QCOMPARE(changedPreset->name(), targetPreset->name());

    qDeleteAll(presets);
}

void KisViewSignalsTest::testForegroundColorChanged()
{
    QScopedPointer<ManagedColor> targetColor(ManagedColor::fromQColor(QColor(12, 34, 56, 255), m_view->canvas()));
    QVERIFY(targetColor);

    QSignalSpy spy(m_view, SIGNAL(foregroundColorChanged()));
    QVERIFY(spy.isValid());

    m_view->setForeGroundColor(targetColor.data());
    QApplication::processEvents();

    QVERIFY(spy.count() >= 1);

    QScopedPointer<ManagedColor> changedColor(m_view->foregroundColor());
    QVERIFY(changedColor);
    QVERIFY(*changedColor == *targetColor);
}

void KisViewSignalsTest::testBackgroundColorChanged()
{
    QScopedPointer<ManagedColor> targetColor(ManagedColor::fromQColor(QColor(78, 90, 123, 255), m_view->canvas()));
    QVERIFY(targetColor);

    QSignalSpy spy(m_view, SIGNAL(backgroundColorChanged()));
    QVERIFY(spy.isValid());

    m_view->setBackGroundColor(targetColor.data());
    QApplication::processEvents();

    QVERIFY(spy.count() >= 1);

    QScopedPointer<ManagedColor> changedColor(m_view->backgroundColor());
    QVERIFY(changedColor);
    QVERIFY(*changedColor == *targetColor);
}

KISTEST_MAIN(KisViewSignalsTest)
