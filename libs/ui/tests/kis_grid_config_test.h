/*
 *  SPDX-FileCopyrightText: 2016 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_GRID_CONFIG_TEST_H
#define __KIS_GRID_CONFIG_TEST_H

#include <QtTest>

class KisGridConfigTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testGridConfig();
    void testGuidesConfig();
};

#endif /* __KIS_GRID_CONFIG_TEST_H */
