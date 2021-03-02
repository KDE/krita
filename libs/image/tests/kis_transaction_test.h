/*
 *  SPDX-FileCopyrightText: 2007 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_TRANSACTION_TEST_H
#define KIS_TRANSACTION_TEST_H

#include <simpletest.h>

class KisTransactionTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:

    void testUndo();
    void testRedo();
    void testDeviceMove();

    void testUndoWithUnswitchedFrames();
};

#endif

