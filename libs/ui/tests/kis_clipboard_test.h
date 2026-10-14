/*
 *  SPDX-FileCopyrightText: 2011 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_CLIPBOARD_TEST_H
#define __KIS_CLIPBOARD_TEST_H

#include <simpletest.h>

class KisClipboardTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testLoadAdaptedFromLayers_data();
    void testLoadAdaptedFromLayers();

    void testLoadFromLayersStandalone_data();
    void testLoadFromLayersStandalone();

    void testLoadFromPngUrl();

    void testLoadFromKraUrl_data();
    void testLoadFromKraUrl();

    void testLoadFromPngClipboard();

    void testLoadFromCloneLayer_data();
    void testLoadFromCloneLayer();

    void testCopyLayerWithMask_data();
    void testCopyLayerWithMask();
};

#endif /* __KIS_CLIPBOARD_TEST_H */
