/*
 *  SPDX-FileCopyrightText: 2007 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_EFFECT_MASK_TESTER_H
#define KIS_EFFECT_MASK_TESTER_H

#include <simpletest.h>

class KisEffectMaskTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:

    void testCreation();
    void testSerialisation();
    void testApplication();
    void testCaching();
};

#endif

