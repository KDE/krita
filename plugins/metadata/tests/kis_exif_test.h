/*
 * SPDX-FileCopyrightText: 2009 Cyrille Berger <cberger@cberger.net>
 * SPDX-FileCopyrightText: 2021 L. E. Segovia <amy@amyspark.me>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef _KIS_EXIV2_TEST_H_
#define _KIS_EXIV2_TEST_H_

#include <QObject>

class KisExifTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testExifLoader();
};

#endif
