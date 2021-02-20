/*
 *  SPDX-FileCopyrightText: 2010 Cyrille Berger <cberger@cberger.net>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef _TEST_KO_COLOR_SPACE_REGISTRY_H_
#define _TEST_KO_COLOR_SPACE_REGISTRY_H_

#include <QObject>

class TestBaseColorSpaceRegistry : public QObject
{
    Q_OBJECT
public:
    TestBaseColorSpaceRegistry();
private Q_SLOTS:
    void testLab16();
    void testRgb8();
    void testRgb16();
    void testProfileByUniqueId();
};

#endif
