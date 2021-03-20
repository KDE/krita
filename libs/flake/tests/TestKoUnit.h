/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2010 Thomas Zander <zander@kde.org>
 * SPDX-FileCopyrightText: 2012 Friedrich W. H. Kossebau <kossebau@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef TESTKOUNIT_H
#define TESTKOUNIT_H

// Qt
#include <QObject>

class TestKoUnit : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testSimpleConstructor();
    void testConstructor_data();
    void testConstructor();
    void testPixelConstructor();
    void testAssignOperator_data();
    void testAssignOperator();
    void testVariant();
    void testFromSymbol_data();
    void testFromSymbol();
    void testListForUi_data();
    void testListForUi();
    void testToUserValue_data();
    void testToUserValue();
};

#endif
