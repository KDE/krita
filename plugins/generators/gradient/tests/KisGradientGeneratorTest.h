/*
 * KDE. Krita Project.
 *
 * SPDX-FileCopyrightText: 2026 Dmitry Kazakov <dimula73@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISGRADIENTGENERATORTEST_H
#define KISGRADIENTGENERATORTEST_H

#include <simpletest.h>

class KisGradientGeneratorTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void testDithering_data();
    void testDithering();
};

#endif /* KISGRADIENTGENERATORTEST_H */
