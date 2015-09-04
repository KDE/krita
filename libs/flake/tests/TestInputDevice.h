/*
 *  This file is part of Calligra tests
 *
 *  Copyright (C) 2011 Hanna Skott <hannaetscott@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
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
