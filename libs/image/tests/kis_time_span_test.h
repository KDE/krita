/*
 *  SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef _KIS_TIME_SPAN_TEST_H
#define _KIS_TIME_SPAN_TEST_H

#include <QtTest>

class KisTimeSpanTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testCreationMethods();
    void testTimeSpanOperators();
};

#endif /* _KIS_TIME_SPAN_TEST_H */
