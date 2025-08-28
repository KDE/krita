/*
 *  SPDX-FileCopyrightText: 2025 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef TESTICCFROMCOLORIMETRYCONVERSION_H
#define TESTICCFROMCOLORIMETRYCONVERSION_H

#include <QObject>

class TestIccFromColorimetryConversion : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testRequestConstruction_data();
    void testRequestConstruction();

    void testRequestConstructionCustomPrimaries();

    void testRequestConstructionCustomGamma_data();
    void testRequestConstructionCustomGamma();

    void testProfileConstruction_data();
    void testProfileConstruction();

    void testProfileConstructionCustomPrimaries();
};

#endif
