/*
 *  SPDX-FileCopyrightText: 2026 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISCONTENTLIGHTLEVELPROCESSINGVISITORTEST_H
#define KISCONTENTLIGHTLEVELPROCESSINGVISITORTEST_H

#include <simpletest.h>

class KisContentLightLevelProcessingVisitorTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testRgbColorSpace_data();
    void testRgbColorSpace();
    void testCmykColorSpace_data();
    void testCmykColorSpace();
    void testEmptyDocument();
};

#endif // KISCONTENTLIGHTLEVELPROCESSINGVISITORTEST_H
