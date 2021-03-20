/*
 *  SPDX-FileCopyrightText: 2007 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KO_PROPERTIES_TEST_H
#define KO_PROPERTIES_TEST_H

#include <QObject>

class KoPropertiesTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:

    void testDeserialization();
    void testRoundTrip();
    void testProperties();
    void testPassAround();
};

#endif

