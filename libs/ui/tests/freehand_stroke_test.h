/*
 *  SPDX-FileCopyrightText: 2011 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __FREEHAND_STROKE_TEST_H
#define __FREEHAND_STROKE_TEST_H

#include <simpletest.h>


class FreehandStrokeTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testAutoBrushStroke();
    void testHatchingStroke();
    void testColorSmudgeStroke();
    void testAutoTextured17();
    void testAutoTextured38();
    void testMixDullCompositioning();

    void testAutoBrushStrokeLod();
    void testPredefinedBrushStrokeLod();
};

#endif /* __FREEHAND_STROKE_TEST_H */
