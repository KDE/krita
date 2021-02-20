/*
 *  SPDX-FileCopyrightText: 2016 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_SHAPE_COMMANDS_TEST_H
#define __KIS_SHAPE_COMMANDS_TEST_H

#include <QtTest>

class KisShapeCommandsTest : public QObject
{
    Q_OBJECT
    void testResizeShape(bool normalizeGroup);

private Q_SLOTS:
    void testGrouping();
    void testResizeShape();
    void testResizeShapeNormalized();

    void testResizeNullShape();
    void testResizeNullShapeGlobal();
    void testScaleNullShape();
    void testScaleNullShapeCovered();
    void testScaleNullShapeGlobal();
};

#endif /* __KIS_SHAPE_COMMANDS_TEST_H */
