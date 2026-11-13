/*
 *  SPDX-FileCopyrightText: 2026 Tenzin Rangdol <tenzindraws@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_BRUSH_STROKE_PREVIEW_CACHE_TEST_H
#define KIS_BRUSH_STROKE_PREVIEW_CACHE_TEST_H

#include <simpletest.h>

class KisBrushStrokePreviewCacheTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void testPlaceholderForInvalidPreset();
    void testScaledCacheInvalidation();
    void testDiskCacheFallback();
};

#endif // KIS_BRUSH_STROKE_PREVIEW_CACHE_TEST_H
