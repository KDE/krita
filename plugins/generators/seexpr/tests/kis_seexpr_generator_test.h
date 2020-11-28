/*
 * This file is part of Krita
 *
 * SPDX-FileCopyrightText: 2020 L. E. Segovia <amy@amyspark.me>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_SEEXPR_GENERATOR_TEST_H
#define KIS_SEEXPR_GENERATOR_TEST_H

#include <QtTest>

class KisSeExprGeneratorTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void testGenerationFromScript();
    void testGenerationFromKoResource();
};

#endif
