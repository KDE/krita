/*
 *  SPDX-FileCopyrightText: 2007 Boudewijn Rempt boud @valdyas.org
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_SHARED_PTR_TEST_H
#define KIS_SHARED_PTR_TEST_H

#include <QtTest>

class KisSharedPtrTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:

    void testRefTwoSharedPointersOneInstance();
    void testCopy();
    void testCopy2();
    void testCopy0();
    void testClear();
    void testWeakSP();
    void testBoolOnInvalidWeakPointer();
    void testInvalidWeakSPAssignToSP();
    void testInvalidWeakSPToSPCopy();
    void testWeakSPAssignToWeakSP();
    void testWeakSPToWeakSPCopy();
    void testRestrictedPointer();
    void testRestrictedPointerNoBackward();
};

#endif
