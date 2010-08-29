/*
 *  This file is part of KOffice tests
 *
 *  Copyright (C) 2006-2010 Thomas Zander <zander@kde.org>
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
#ifndef TESTIMAGECOLLECTION_H
#define TESTIMAGECOLLECTION_H

#include <QtTest/QtTest>

class TestImageCollection : public QObject
{
    Q_OBJECT
private slots:
    // tests
    void testGetImageImage();
    void testGetExternalImage();
    void testGetImageStore();
    void testInvalidImageData();

    // imageData tests
    void testImageDataAsSharedData();
    void testPreload1();
    void testPreload2();
    void testPreload3();
    void testSameKey();
    void testIsValid();
};

#endif /* TESTIMAGECOLLECTION_H */
