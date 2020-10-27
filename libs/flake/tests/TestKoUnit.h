/* This file is part of the KDE project
 * Copyright (C) 2010 Thomas Zander <zander@kde.org>
 * Copyright 2012 Friedrich W. H. Kossebau <kossebau@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
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
