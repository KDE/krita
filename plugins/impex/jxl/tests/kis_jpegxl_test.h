/*
 * SPDX-FileCopyrightText: 2022 L. E. Segovia <amy@amyspark.me>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef _KIS_JPEG_TEST_H_
#define _KIS_JPEG_TEST_H_

#include <simpletest.h>

class KisJPEGXLTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testAnimation();
    void testAnimationWithTail();
    void testFiles();
    void testHDR();
    void testCmykWithLayers();
    void testSaveRgbaColorSpace();
    void testSaveGreyAColorSpace();
    void testSaveCmykAColorSpace();
    void testImportIncorrectFormat();

#ifndef Q_OS_WIN
private Q_SLOTS:
    void testImportFromWriteonly();
    void testExportToReadonly();
#endif
};

#endif
