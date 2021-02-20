/*
 *  This file is part of Calligra tests
 *
 *  SPDX-FileCopyrightText: 2006-2010 Thomas Zander <zander@kde.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef TestShapeContainer_H
#define TestShapeContainer_H

#include <QObject>

class TestShapeContainer : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    // tests
    void testModel();
    void testSetParent();
    void testSetParent2();
    void testScaling();
    void testScaling2();
};

#endif
