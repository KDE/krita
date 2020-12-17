/*
 *  SPDX-FileCopyrightText: 2012 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_IMAGEPIPE_BRUSH_TEST_H
#define __KIS_IMAGEPIPE_BRUSH_TEST_H

#include <QtTest>

#include <kis_imagepipe_brush.h>

class KisImagePipeBrushTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testLoading();
    void testChangingBrushes();
    void testSimpleDabApplication();
    void testColoredDab();
    void testColoredDabWash();
    void testTextBrushNoPipes();
    void testTextBrushPiped();

private:
    void checkConsistency(KisImagePipeBrushSP brush);
};

#endif /* __KIS_IMAGEPIPE_BRUSH_TEST_H */
