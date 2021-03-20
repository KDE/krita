/*
 *  This file is part of Calligra tests
 *
 *  SPDX-FileCopyrightText: 2006-2010 Thomas Zander <zander@kde.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef TestPosition_H
#define TestPosition_H

#include <QObject>

class KoShape;
class KoShapeContainer;

class TestPosition : public QObject
{
    Q_OBJECT
public:
    TestPosition();

private Q_SLOTS:
    void init(); // will be called before each testfunction is executed.
    void cleanup();  // will be called after each testfunction is executed.

    // tests
    void testBasePosition();
    void testAbsolutePosition();
    void testSetAbsolutePosition();
    void testSetAbsolutePosition2();
    void testSetAndGetRotation();

private:
    void resetValues();
    void resetValues(KoShape *shape);

    // vars
    KoShape *shape1, *shape2, *childShape1, *childShape2;
    KoShapeContainer *container, *container2;
};

#endif
