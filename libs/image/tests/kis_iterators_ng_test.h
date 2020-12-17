/*
 *  SPDX-FileCopyrightText: 2007 Boudewijn Rempt <boud@kde.org>
 *  SPDX-FileCopyrightText: 2010 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_ITERATOR_NG_TEST_H
#define KIS_ITERATOR_NG_TEST_H

#include <QtTest>

class KoColorSpace;

class KisIteratorNGTest : public QObject
{
    Q_OBJECT

private:
    void allCsApplicator(void (KisIteratorNGTest::* funcPtr)(const KoColorSpace*cs));

    void justCreation(const KoColorSpace * cs);
    void vLineIter(const KoColorSpace * cs);
    void writeBytes(const KoColorSpace * cs);
    void fill(const KoColorSpace * cs);
    void sequentialIter(const KoColorSpace * colorSpace);
    void hLineIter(const KoColorSpace * cs);
    void randomAccessor(const KoColorSpace * cs);

private Q_SLOTS:
    void justCreation();
    void vLineIter();
    void writeBytes();
    void fill();
    void sequentialIter();
    void sequentialIteratorWithProgress();
    void sequentialIteratorWithProgressIncomplete();
    void hLineIter();
    void randomAccessor();
};

#endif

