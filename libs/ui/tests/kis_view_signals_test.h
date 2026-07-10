/*
 *  SPDX-FileCopyrightText: 2026 Moritz Staudinger
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_VIEW_SIGNALS_TEST_H
#define KIS_VIEW_SIGNALS_TEST_H

#include <QObject>

class KisDocument;
class KisMainWindow;
class KisView;
class KisViewManager;
class View;

class KisViewSignalsTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();

    void testCurrentToolChanged();
    void testCurrentBrushPresetChanged();
    void testForegroundColorChanged();
    void testBackgroundColorChanged();

private:
    KisDocument *m_document = nullptr;
    KisMainWindow *m_mainWindow = nullptr;
    KisView *m_kisView = nullptr;
    KisViewManager *m_viewManager = nullptr;
    View *m_view = nullptr;
};

#endif
