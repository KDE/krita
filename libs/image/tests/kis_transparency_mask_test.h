/*
 *  SPDX-FileCopyrightText: 2007 Boudewijn Rempt boud @valdyas.org
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_TRANSPARENCY_MASK_TEST_H
#define KIS_TRANSPARENCY_MASK_TEST_H

#include <simpletest.h>

class KisTransparencyMaskTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:

    void testApply();
    void testMoveParentLayer();
    void testMoveMaskItself();
};

#endif
