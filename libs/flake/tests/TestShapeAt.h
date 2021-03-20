/*
 *  This file is part of Calligra tests
 *
 *  SPDX-FileCopyrightText: 2006-2010 Thomas Zander <zander@kde.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef TestShapeAt_H
#define TestShapeAt_H

#include <QObject>

class TestShapeAt : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    // tests
    void test();
    void testShadow();

};

#endif
