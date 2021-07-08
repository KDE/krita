/*
 * SPDX-FileCopyrightText: 2009 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef _COMPRESSION_TEST_H_
#define _COMPRESSION_TEST_H_

#include <simpletest.h>

class CompressionTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:

    void testCompressionRLE();
    void testCompressionZIP();
    void testCompressionUncompressed();
};

#endif
