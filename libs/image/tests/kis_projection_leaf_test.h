/*
 *  SPDX-FileCopyrightText: 2015 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_PROJECTION_LEAF_TEST_H
#define __KIS_PROJECTION_LEAF_TEST_H

#include <QtTest/QtTest>

class KisProjectionLeafTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void test();
    void testPassThrough();
    void testNestedPassThrough();
    void testSkippedSelectionMasks();

    void testSelectionMaskOverlay();
};

#endif /* __KIS_PROJECTION_LEAF_TEST_H */
