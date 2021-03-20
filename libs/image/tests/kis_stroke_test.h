/*
 *  SPDX-FileCopyrightText: 2011 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_STROKE_TEST_H
#define __KIS_STROKE_TEST_H

#include <simpletest.h>


class KisStrokeTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testRegularStroke();
    void testCancelStrokeCase1();
    void testCancelStrokeCase2and3();
    void testCancelStrokeCase5();
    void testCancelStrokeCase4();
    void testCancelStrokeCase6();
};

#endif /* __KIS_STROKE_TEST_H */
