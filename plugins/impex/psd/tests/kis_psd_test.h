/*
 * SPDX-FileCopyrightText: 2009 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef _KIS_PSD_TEST_H_
#define _KIS_PSD_TEST_H_

#include <simpletest.h>

class KisPSDTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testFiles();
    void testOpening();
    void testTransparencyMask();
    void testOpenGrayscaleMultilayered();
    void testOpenGroupLayers();
    void testOpenLayerStyles();

    void testOpenLayerStylesWithPattern();
    void testOpenLayerStylesWithPatternMulti();

    void testSaveLayerStylesWithPatternMulti();

    void testOpeningFromOpenCanvas();
    void testOpeningAllFormats();
    void testSavingAllFormats();


    void testImportFromWriteonly();
    void testExportToReadonly();
    void testImportIncorrectFormat();
};

#endif
