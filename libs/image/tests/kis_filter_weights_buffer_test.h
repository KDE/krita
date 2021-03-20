/*
 *  SPDX-FileCopyrightText: 2012 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_FILTER_WEIGHTS_BUFFER_TEST_H
#define __KIS_FILTER_WEIGHTS_BUFFER_TEST_H

#include <simpletest.h>

class KisFilterWeightsBufferTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testTriangle();
    void testHermite();
    void testBicubic();
    void testBox();
    void testBell();
    void testBSpline();
    void testLanczos3();
    void testMitchell();
};

#endif /* __KIS_FILTER_WEIGHTS_BUFFER_TEST_H */
