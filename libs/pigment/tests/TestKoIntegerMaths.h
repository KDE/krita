/*
 *  SPDX-FileCopyrightText: 2005 Adrian Page <adrian@pagenet.plus.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef _TESTKOINTEGERMATHS_H
#define _TESTKOINTEGERMATHS_H

#include <QObject>

class TestKoIntegerMaths : public QObject
{
    Q_OBJECT

private Q_SLOTS:

    void UINT8Tests();
    void UINT16Tests();
    void conversionTests();
};

#endif

