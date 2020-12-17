/*
 *  SPDX-FileCopyrightText: 2015 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __TIMELINE_MODEL_TEST_H
#define __TIMELINE_MODEL_TEST_H

#include <QtTest>
#include "empty_nodes_test.h"

class KisDocument;
class KisNameServer;
class KisShapeController;
class KisNodeDisplayModeAdapter;


class TimelineModelTest : public QObject, public TestUtil::EmptyNodesTest
{
    Q_OBJECT
private Q_SLOTS:
    void init();
    void cleanup();

    void testConverter();
    void testModel();
    void testView();
    void testOnionSkins();

private Q_SLOTS:
    void setCurrentTime(int time);
    void setCurrentLayer(int row);
    void slotGuiChangedNode(KisNodeSP node);

    void slotBang();

Q_SIGNALS:
    void sigRequestNodeChange(KisNodeSP node);

private:
    KisDocument *m_doc;
    KisNameServer *m_nameServer;
    KisShapeController *m_shapeController;
    KisNodeDisplayModeAdapter *m_displayModeAdapter;
};

#endif /* __TIMELINE_MODEL_TEST_H */
