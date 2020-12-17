/*
 *  SPDX-FileCopyrightText: 2012 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_NODE_MANAGER_TEST_H
#define __KIS_NODE_MANAGER_TEST_H

#include <QtTest>

class KisNodeManagerTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testMirrorXPaintNode();
    void testMirrorYPaintNode();

    void testMirrorShapeNode();

    void testConvertCloneToPaintLayer();
    void testConvertCloneToSelectionMask();

    void testConvertBlurToSelectionMask();
};

#endif /* __KIS_NODE_MANAGER_TEST_H */
