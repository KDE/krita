/*
 *  SPDX-FileCopyrightText: 2015 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_ASL_PARSER_TEST_H
#define __KIS_ASL_PARSER_TEST_H

#include <QtTest/QtTest>

class KisAslParserTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void test();
    void testWithCallbacks();
    void testASLXMLWriter();
    void testWritingGradients();

    void testASLWriter();

    void testParserWithPatterns();
};

#endif /* __KIS_ASL_PARSER_TEST_H */
