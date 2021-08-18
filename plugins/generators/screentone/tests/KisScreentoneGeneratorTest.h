/*
 * KDE. Krita Project.
 *
 * SPDX-FileCopyrightText: 2020 Deif Lou <ginoba@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISSCREENTONEGENERATORTEST_H
#define KISSCREENTONEGENERATORTEST_H

#include <simpletest.h>

class KisScreentoneGeneratorTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void testGenerate01();
    void testGenerate02();
    void testGenerate03();
    void testGenerate04();
    void testGenerate05();
    void testGenerate06();
    void testGenerate07();
    void testGenerate08();
    void testGenerate09();
};

#endif
