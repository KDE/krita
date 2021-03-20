/*
 *  SPDX-FileCopyrightText: 2015 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_DOM_UTILS_TEST_H
#define __KIS_DOM_UTILS_TEST_H

#include <simpletest.h>

class KisDomUtilsTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:

    void testC2C();
    void testG2G();
    void testR2R();

    void testC2G();
    void testR2G();
    void testG2C();
    void testG2R();

    void testIntegralType();
};

#endif /* __KIS_DOM_UTILS_TEST_H */
