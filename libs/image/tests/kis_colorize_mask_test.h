/*
 *  SPDX-FileCopyrightText: 2016 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_COLORIZE_MASK_TEST_H
#define __KIS_COLORIZE_MASK_TEST_H

#include <QtTest>

class KisColorizeMaskTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void test();
    void testCrop();
};

#endif /* __KIS_COLORIZE_MASK_TEST_H */
