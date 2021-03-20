/*
 *  SPDX-FileCopyrightText: 2007 Boudewijn Rempt boud @valdyas.org
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_BRUSH_TEST_H
#define KIS_BRUSH_TEST_H

#include <simpletest.h>

class KisGbrBrushTest : public QObject
{
    Q_OBJECT

    // XXX disabled until I figure out why they don't work from here, while the brushes do work from Krita
    void testMaskGenerationSingleColor();
    void testMaskGenerationDevColor();

private Q_SLOTS:

    void testImageGeneration();

    void benchmarkPyramidCreation();
    void benchmarkScaling();
    void benchmarkRotation();
    void benchmarkMaskScaling();

    void testPyramidLevelRounding();
    void testPyramidDabTransform();

    void testQPainterTransformationBorder();
};

#endif
