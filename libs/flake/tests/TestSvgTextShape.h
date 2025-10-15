/*
 *  SPDX-FileCopyrightText: 2025 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef TESTSVGTEXTSHAPE_H
#define TESTSVGTEXTSHAPE_H


#include <simpletest.h>
#include <QObject>

class TestSvgTextShape : public QObject
{
    Q_OBJECT
private Q_SLOTS:

    void testSetTextOnShape_data();
    void testSetTextOnShape();

    void testRemoveShapeFromText_data();
    void testRemoveShapeFromText();

    void testSetSize_data();
    void testSetSize();

    void testToggleShapeType();

    void testReorderShapesInside_data();
    void testReorderShapesInside();

};

#endif // TESTSVGTEXTSHAPE_H
