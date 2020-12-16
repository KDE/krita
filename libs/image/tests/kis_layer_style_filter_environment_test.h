/*
 *  SPDX-FileCopyrightText: 2015 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_LAYER_STYLE_FILTER_ENVIRONMENT_TEST_H
#define __KIS_LAYER_STYLE_FILTER_ENVIRONMENT_TEST_H

#include <QtTest/QtTest>

class KisLayerStyleFilterEnvironmentTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testRandomSelectionCaching();
    void benchmarkRandomSelectionGeneration();
};

#endif /* __KIS_LAYER_STYLE_FILTER_ENVIRONMENT_TEST_H */
