/*
*  SPDX-FileCopyrightText: 2025 Dmitry Kazakov <dimula73@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KO_PLUGIN_LOADER_TEST_H
#define KO_PLUGIN_LOADER_TEST_H

#include <QObject>

class KoPluginLoaderTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();

    void testLoadSinglePlugin_data();
    void testLoadSinglePlugin();

    void testLoadAll_data();
    void testLoadAll();
};

#endif // KO_PLUGIN_LOADER_TEST_H