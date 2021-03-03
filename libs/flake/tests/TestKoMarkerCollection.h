/*
 *  SPDX-FileCopyrightText: 2017 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef TESTKOMARKERCOLLECTION_H
#define TESTKOMARKERCOLLECTION_H

#include <simpletest.h>

class TestKoMarkerCollection  : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testLoadMarkersFromFile();
    void testDeduplication();
    void testMarkerBounds();
};

#endif // TESTKOMARKERCOLLECTION_H
