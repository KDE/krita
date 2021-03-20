/*
 *  SPDX-FileCopyrightText: 2007 Boudewijn Rempt <boud@kde.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_ITERATOR_TEST_H
#define KIS_ITERATOR_TEST_H

#include <simpletest.h>

class KoColorSpace;

class KisIteratorTest : public QObject
{
    Q_OBJECT

    /// re-activate once bug https://bugs.kde.org/show_bug.cgi?id=276198 is fixed.
    void stressTest();

private:
    void allCsApplicator(void (KisIteratorTest::* funcPtr)(const KoColorSpace*cs));

    void vLineIter(const KoColorSpace * cs);
    void writeBytes(const KoColorSpace * cs);
    void fill(const KoColorSpace * cs);
    void hLineIter(const KoColorSpace * cs);
    void randomAccessor(const KoColorSpace * cs);
    void repeatHLineIter(const KoColorSpace * cs);
    void repeatVLineIter(const KoColorSpace * cs);

private Q_SLOTS:

    void vLineIter();
    void writeBytes();
    void fill();
    void hLineIter();
    void randomAccessor();
    void repeatHLineIter();
    void repeatVLineIter();

};

#endif

