/*
 *  This file is part of Calligra tests
 *
 *  SPDX-FileCopyrightText: 2011 Hanna Skott <hannaetscott@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef TESTINPUTDEVICE_H
#define TESTINPUTDEVICE_H

#include <QObject>

class TestInputDevice : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    // tests
    void testTabletConstructor();
    void testNoParameterConstructor();
    void testConstructorWithSingleReference();
    void testEqualityCheckOperator();
    void testDevice();
    void testPointer();
    void testMouse();
    void testStylus();
    void testEraser();
    
};

#endif /* TESTINPUTDEVICE_H */
