/*
 * Copyright (C) 2007 Boudewijn Rempt <boud@kde.org>
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

#ifndef KISSHAPECONTROLLER_TEST_H
#define KISSHAPECONTROLLER_TEST_H

#include <QtTest/QtTest>

class KisDoc2;
class KisNameServer;
class KisImage;
class KisShapeController;

class KisShapeControllerTest : public QObject
{
    Q_OBJECT

private slots:

    void initTestCase();
    void cleanupTestCase();

    // tests
    void testSetImage();
    void testAddShape();
    void testRemoveShape();
    void testSetInitialShapeForView();
    void testShapeForLayer();

private:

    KisDoc2 * m_doc;
    KisNameServer * m_nameServer;
    KisImage * m_image;
    KisShapeController * m_shapeController;

};

#endif

