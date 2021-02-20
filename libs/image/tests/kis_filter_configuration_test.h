/*
 *  SPDX-FileCopyrightText: 2006 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_FILTER_CONFIGURATION_TESTER_H
#define KIS_FILTER_CONFIGURATION_TESTER_H

#include <QtTest>

class KisFilterConfigurationTest : public QObject
{

    Q_OBJECT

private Q_SLOTS:

    void testCreation();
    void testRoundTrip();
    void testSetGetProperty();
};

#endif

