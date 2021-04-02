/*
 *  SPDX-FileCopyrightText: 2005 Adrian Page <adrian@pagenet.plus.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */


#ifndef KORGBCOLORSPACETESTER_H
#define KORGBCOLORSPACETESTER_H

#include <QObject>

class KoRgbU8ColorSpaceTester : public QObject
{
    Q_OBJECT
    void testCompositeOps();
private Q_SLOTS:
    void testBasics();
    void testMixColors();
    void testMixColorsAverage();
    void testCompositeOpsWithChannelFlags();

    void testScaler();
};

#endif

