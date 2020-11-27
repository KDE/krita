/*
 *  SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef TESTKOCOLORSPACEMATHS_H
#define TESTKOCOLORSPACEMATHS_H

#include <QObject>

class TestKoColorSpaceMaths : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testColorSpaceMathsTraits();
    void testScaleToA();
};

#endif
