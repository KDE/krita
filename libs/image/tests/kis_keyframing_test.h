/*
 *  SPDX-FileCopyrightText: 2015 Jouni Pentikäinen <joupent@gmail.com>
 *  SPDX-FileCopyrightText: 2020 Emmet O 'Neill <emmetoneill.pdx@gmail.com>
 *  SPDX-FileCopyrightText: 2020 Eoin O 'Neill <eoinoneill1991@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_KEYFRAMING_TEST_H
#define KIS_KEYFRAMING_TEST_H

#include <simpletest.h>
#include "KoColor.h"


class KisKeyframingTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();

    void testChannelSignals();

    void testRasterChannel();
    void testRasterFrameFetching();
    void testRasterUndoRedo();
    void testFirstFrameOperations();
    void testInterChannelMovement();

    void testScalarChannel();
    void testScalarValueInterpolation();
    void testScalarChannelUndoRedo();
    void testScalarAffectedFrames();
    void testChangeOfScalarLimits();

private:
    const KoColorSpace *cs;

    quint8* red;
    quint8* green;
    quint8* blue;
};

#endif

