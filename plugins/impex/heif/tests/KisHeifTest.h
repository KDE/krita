/*
 * SPDX-FileCopyrightText: 2019 Agata Cacko <cacko.azh@gmail.com>
 * SPDX-FileCopyrightText: 2021 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef _KIS_HEIF_TEST_H_
#define _KIS_HEIF_TEST_H_

#include <simpletest.h>

class KisHeifTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:

    void testImportFromWriteonly();
    void testExportToReadonly();
    void testImportIncorrectFormat();

    void testSaveMonochrome(int bitDepth);
    void testSaveRGB(int bitDepth);
    void testLoadMonochrome(int bitDepth);
    void testLoadRGB(int bitDepth);

    void testSaveHDR();
    void testLoadHDR();

    void testImages();
};

#endif // _KIS_HEIF_TEST_H_

