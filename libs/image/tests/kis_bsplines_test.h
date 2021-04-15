/*
 *  SPDX-FileCopyrightText: 2014 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_BSPLINES_TEST_H
#define __KIS_BSPLINES_TEST_H

#include <simpletest.h>

class KisBSplinesTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void test1D();
    void testEmpty1D();

    void test2D();
    void testEmpty2D();

    void testNU2D();
};

#endif /* __KIS_BSPLINES_TEST_H */
