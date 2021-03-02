/*
 *  SPDX-FileCopyrightText: 2018 Iván Santa María <ghevan@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_MASK_SIMILARITY_TEST
#define KIS_MASK_SIMILARITY_TEST

#include <simpletest.h>

class KisMaskSimilarityTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:

    void testCircleMask();
    void testGaussCircleMask();
    void testSoftCircleMask();

    void testRectMask();
    void testGaussRectMask();
    void testSoftRectMask();
};

#endif
