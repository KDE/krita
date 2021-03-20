/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2006 Thorsten Zachmann <zachmann@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/
#ifndef TESTPATHSHAPE_H
#define TESTPATHSHAPE_H

#include <QObject>

class TestPathShape : public QObject

{
    Q_OBJECT
private Q_SLOTS:
    void close();
    void moveTo();
    void normalize();

    void pathPointIndex();
    void pointByIndex();
    void segmentByIndex();
    void pointCount();
    void subpathPointCount();
    void isClosedSubpath();
    void insertPoint();
    void removePoint();
    void splitAfter();
    void join();
    void moveSubpath();
    void openSubpath();
    void closeSubpath();
    void openCloseSubpath();
    void reverseSubpath();
    void removeSubpath();
    void addSubpath();
    void closeMerge();

    void koPathPointDataLess();
};

#endif // TESTPATHSHAPE_H
