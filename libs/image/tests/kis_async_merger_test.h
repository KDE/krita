/*
 *  SPDX-FileCopyrightText: 2009 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_ASYNC_MERGER_TEST_H
#define KIS_ASYNC_MERGER_TEST_H

#include <QtTest>

class KisAsyncMergerTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void init();

    void testMerger();
    void debugObligeChild();
    void testFullRefreshWithClones();
    void testSubgraphingWithoutUpdatingParent();

    void testFullRefreshGroupWithMask();
    void testFullRefreshGroupWithStyle();
    void testFullRefreshGroupWithMaskAndStyle();
    void testFullRefreshAdjustmentWithMask();
    void testFullRefreshAdjustmentWithStyle();

    void testFilterMaskOnFilterLayer();

};

#endif /* KIS_ASYNC_MERGER_TEST_H */

