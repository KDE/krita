/*
 *  SPDX-FileCopyrightText: 2014 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __TEST_SAVE_LOAD_TRANSFORM_ARGS_H
#define __TEST_SAVE_LOAD_TRANSFORM_ARGS_H

#include <QtTest>

class TestSaveLoadTransformArgs : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testFreeTransform();
    void testWarp();
    void testLiquify();
    void testCage();
};

#endif /* __TEST_SAVE_LOAD_TRANSFORM_ARGS_H */
