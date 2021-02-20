/*
 *  SPDX-FileCopyrightText: 2007 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_SERIALIZABLE_CONFIGURATION_TEST_H
#define KIS_SERIALIZABLE_CONFIGURATION_TEST_H

#include <QtTest>

#include "kis_cubic_curve.h"
#include "kis_properties_configuration.h"

class KisPropertiesConfigurationTest : public QObject
{
    Q_OBJECT
public:
    KisPropertiesConfigurationTest();
private Q_SLOTS:
    void testSetGet();
    void testSerialization();
    void testDefaultValues();
    void testNotSavedValues();
    void testCopy();
    void testGetColor();
    void testLists();

private:
    KisPropertiesConfigurationSP createConfig();
    void testConfig(KisPropertiesConfigurationSP config);
private:
    int v1;
    QString v2;
    double v3;
    bool v4;
    KisCubicCurve v5;
};

#endif

