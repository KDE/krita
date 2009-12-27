/* This file is part of the KDE project
   Copyright (C) 2007 Thorsten Zachmann <zachmann@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/
#include "TestPathTool.h"

#include <QPainterPath>
#include "../KoPathShape.h"
#include "../tools/KoPathTool.h"
#include "../tools/KoPathToolSelection.h"
#include "../KoPathPointData.h"
#include <MockShapes.h>

void TestPathTool::koPathPointSelection_selectedSegmentsData()
{
    KoPathShape path1;
    KoPathPoint *point11 = path1.moveTo(QPointF(10, 10));
    KoPathPoint *point12 = path1.lineTo(QPointF(20, 10));
    KoPathPoint *point13 = path1.lineTo(QPointF(20, 20));
    KoPathPoint *point14 = path1.lineTo(QPointF(15, 25));
    path1.lineTo(QPointF(10, 20));
    KoPathPoint *point16 = path1.moveTo(QPointF(30, 30));
    path1.lineTo(QPointF(40, 30));
    KoPathPoint *point18 = path1.lineTo(QPointF(40, 40));
    KoPathPoint *point19 = path1.curveTo(QPointF(40, 45), QPointF(30, 45), QPointF(30, 40));
    path1.close();

    KoPathShape path2;
    KoPathPoint *point21 = path2.moveTo(QPointF(100, 100));
    KoPathPoint *point22 = path2.lineTo(QPointF(110, 100));
    KoPathPoint *point23 = path2.lineTo(QPointF(110, 110));

    KoPathShape path3;
    KoPathPoint *point31 = path3.moveTo(QPointF(200, 220));
    KoPathPoint *point32 = path3.lineTo(QPointF(210, 220));
    KoPathPoint *point33 = path3.lineTo(QPointF(220, 220));
    path3.close();

    MockCanvas canvas;
    KoPathTool tool(&canvas);
    QVERIFY(1 == 1);
    KoPathToolSelection pps(&tool);
    pps.add(point11, false);
    pps.add(point12, false);
    pps.add(point13, false);
    pps.add(point14, false);
    pps.add(point16, false);
    pps.add(point18, false);
    pps.add(point19, false);
    pps.add(point21, false);
    pps.add(point22, false);
    pps.add(point23, false);
    pps.add(point31, false);
    pps.add(point32, false);
    pps.add(point33, false);

    QList<KoPathPointData> pd2;
    pd2.append(KoPathPointData(&path1, path1.pathPointIndex(point11)));
    pd2.append(KoPathPointData(&path1, path1.pathPointIndex(point12)));
    pd2.append(KoPathPointData(&path1, path1.pathPointIndex(point13)));
    pd2.append(KoPathPointData(&path1, path1.pathPointIndex(point18)));
    pd2.append(KoPathPointData(&path1, path1.pathPointIndex(point19)));
    pd2.append(KoPathPointData(&path2, path2.pathPointIndex(point21)));
    pd2.append(KoPathPointData(&path2, path2.pathPointIndex(point22)));
    pd2.append(KoPathPointData(&path3, path3.pathPointIndex(point31)));
    pd2.append(KoPathPointData(&path3, path3.pathPointIndex(point32)));
    pd2.append(KoPathPointData(&path3, path3.pathPointIndex(point33)));

    qSort(pd2);

    QList<KoPathPointData> pd1(pps.selectedSegmentsData());
    QVERIFY(pd1 == pd2);
}


QTEST_KDEMAIN(TestPathTool, GUI)
#include "TestPathTool.moc"
