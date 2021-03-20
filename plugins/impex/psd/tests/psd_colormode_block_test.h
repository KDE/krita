/*
 * SPDX-FileCopyrightText: 2009 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef _PSD_COLORMODE_BLOCK_TEST_H_
#define _PSD_COLORMODE_BLOCK_TEST_H_

#include <simpletest.h>

class PSDColorModeBlockTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testCreation();
    void testLoadingRGB();
    void testLoadingIndexed();
};

#endif
