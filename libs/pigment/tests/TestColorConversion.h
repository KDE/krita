/*
 *  SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef TestColorConversion_H
#define TestColorConversion_H

#include <QObject>

class TestColorConversion : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testRGBHSV();
    void testRGBHSL();
};

#endif
