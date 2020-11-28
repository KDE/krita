/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2015 Friedrich W. H. Kossebau <kossebau@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef TESTKOLZF_H
#define TESTKOLZF_H

// Qt
#include <QObject>

class TestKoLZF : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testArrayCompressionEmpty_data();
    void testArrayCompressionEmpty();
    void testArrayCompressionNullPointerInput();
    void testArrayCompressionNullPointerOutput();
    void testArrayDecompressionEmpty_data();
    void testArrayDecompressionEmpty();
    void testArrayDecompressionNullPointerInput();
    void testArrayDecompressionNullPointerOutput();
    void testArrayRoundtripDifferentSizes_data();
    void testArrayRoundtripDifferentSizes();

    void testByteArrayCompressionEmpty();
    void testByteArrayDecompressionEmpty();
    void testByteArrayRoundtripDifferentSizes_data();
    void testByteArrayRoundtripDifferentSizes();
};

#endif
