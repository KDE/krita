/*
 *  SPDX-FileCopyrightText: 2010 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef TEST_FALL_BACK_COLOR_TRANSFORMATION_H_
#define TEST_FALL_BACK_COLOR_TRANSFORMATION_H_

#include <QObject>

class TestFallBackColorTransformation : public QObject
{
    Q_OBJECT

private Q_SLOTS:

    void parametersForward();
};

#endif

