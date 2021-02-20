/*
 * KDE. Krita Project.
 *
 * SPDX-FileCopyrightText: 2020 Deif Lou <ginoba@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISSCREENTONEGENERATORTEST_H
#define KISSCREENTONEGENERATORTEST_H

#include <QtTest>

class KisScreentoneGeneratorTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void testGenerate01();
    void testGenerate02();
};

#endif
