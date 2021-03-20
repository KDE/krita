/*
 *  This file is part of Calligra tests
 *
 *  SPDX-FileCopyrightText: 2006-2010 Thomas Zander <zander@kde.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef TESTSHAPEPAINT_H
#define TESTSHAPEPAINT_H

#include <QObject>

class TestShapePainting : public QObject
{
    Q_OBJECT
private Q_SLOTS:

    void testPaintShape();
    void testPaintHiddenShape();
    void testPaintOrder();
    void testGroupUngroup();
};

#endif
