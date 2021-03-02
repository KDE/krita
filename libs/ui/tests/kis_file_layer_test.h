/*
 *  SPDX-FileCopyrightText: 2015 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_FILE_LAYER_TEST_H
#define __KIS_FILE_LAYER_TEST_H

#include <simpletest.h>

class KisFileLayerTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testFileLayerPlusTransformMaskOffImage();
    void testFileLayerPlusTransformMaskSmallFileBigOffset();
};

#endif /* __KIS_FILE_LAYER_TEST_H */
