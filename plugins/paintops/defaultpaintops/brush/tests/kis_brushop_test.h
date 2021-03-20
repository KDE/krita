/*
 *  SPDX-FileCopyrightText: 2013 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_BRUSHOP_TEST_H
#define __KIS_BRUSHOP_TEST_H

#include <simpletest.h>

class KisBrushOpTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();

    void testRotationMirroring();
    void testRotationMirroringDrawingAngle();
    void testMagicSeven();
};

#endif /* __KIS_BRUSHOP_TEST_H */
