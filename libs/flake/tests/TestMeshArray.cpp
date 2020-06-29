/*
 *  Copyright (c) 2020 Sharaf Zaman <sharafzaz121@gmail.com>
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
#include "TestMeshArray.h"

#include <SvgParser.h>
#include <SvgMeshArray.h>
#include <QtTest/qtest.h>


TestMeshArray::TestMeshArray()
{
}


void TestMeshArray::testSinglePatch()
{
    SvgMeshArray mesharray;
    mesharray.newRow();
    QPointF point(50, 50);
    mesharray.addPatch(patch0, point);

    QList<QPointF> path = {QPointF(50,50), QPointF(75,25), QPointF(125,75), QPointF(150,50)};
    QCOMPARE(mesharray.getPath(SvgMeshPatch::Top, 0, 0), path);
    path = {QPointF(150,50), QPointF(175,75), QPointF(125,125), QPointF(150,150)};
    QCOMPARE(mesharray.getPath(SvgMeshPatch::Right, 0, 0), path);
    path = {QPointF(150,150), QPointF(125,175), QPointF(75,125), QPointF(50,150)};
    QCOMPARE(mesharray.getPath(SvgMeshPatch::Bottom, 0, 0), path);
    path = {QPointF(50,150), QPointF(25,125), QPointF(75,75), QPointF(50,50)};
    QCOMPARE(mesharray.getPath(SvgMeshPatch::Left, 0, 0), path);
}

void TestMeshArray::test_2_by_2_Patch()
{
    SvgMeshArray mesharray;

    QPointF point(50, 50);
    mesharray.newRow();
    mesharray.addPatch(patch0, point);

    point = mesharray.getStop(SvgMeshPatch::Right, 0, 0).point;
    mesharray.addPatch(patch1, point);

    mesharray.newRow();
    point = mesharray.getStop(SvgMeshPatch::Bottom, 0, 0).point;
    mesharray.addPatch(patch2, point);

    point = mesharray.getStop(SvgMeshPatch::Bottom, 0, 1).point;
    mesharray.addPatch(patch3, point);

    QList<QPointF> path = {QPointF(50,50), QPointF(75,25), QPointF(125,75), QPointF(150,50)};
    QCOMPARE(mesharray.getPath(SvgMeshPatch::Top, 0, 0), path);

    // compare the common side Top Bottom, col = 0
    path = {QPointF(50,150), QPointF(75,125), QPointF(125,175), QPointF(150,150)};
    QCOMPARE(mesharray.getPath(SvgMeshPatch::Top, 1, 0), path);
    std::reverse(path.begin(), path.end());
    QCOMPARE(mesharray.getPath(SvgMeshPatch::Bottom, 0, 0), path);

    // compare the common side Top Bottom, col = 1
    path = {QPointF(150,150), QPointF(175,125), QPointF(225,175), QPointF(250,150)};
    QCOMPARE(mesharray.getPath(SvgMeshPatch::Top, 1, 1), path);
    std::reverse(path.begin(), path.end());
    QCOMPARE(mesharray.getPath(SvgMeshPatch::Bottom, 0, 1), path);

    // compare the common side Right-Left, row = 0
    path = {QPointF(150,150), QPointF(125,125), QPointF(175,75), QPointF(150,50)};
    QCOMPARE(mesharray.getPath(SvgMeshPatch::Left, 0, 1), path);
    path = {QPointF(150,150), QPointF(125,125), QPointF(175,75), QPointF(150,50)};
    std::reverse(path.begin(), path.end());
    QCOMPARE(mesharray.getPath(SvgMeshPatch::Right, 0, 0), path);

    // compare the common side Right-Left, row = 1
    path = {QPointF(150,150), QPointF(175,175), QPointF(125,225), QPointF(150,250)};
    QCOMPARE(mesharray.getPath(SvgMeshPatch::Right, 1, 0), path);
    std::reverse(path.begin(), path.end());
    QCOMPARE(mesharray.getPath(SvgMeshPatch::Left, 1, 1), path);

    path = {QPointF(50,250), QPointF(25,225), QPointF(75,175), QPointF(50,150)};
    QCOMPARE(mesharray.getPath(SvgMeshPatch::Left, 1, 0), path);

    path = {QPointF(250,250), QPointF(225,275), QPointF(175,225), QPointF(150,250)};
    QCOMPARE(mesharray.getPath(SvgMeshPatch::Bottom, 1, 1), path);

    path = {QPointF(250,150), QPointF(275,175), QPointF(225,225), QPointF(250,250)};
    QCOMPARE(mesharray.getPath(SvgMeshPatch::Right, 1, 1), path);
}

void TestMeshArray::test_linear_path()
{
    SvgMeshArray mesharray;
    QPointF point(50, 50);
    mesharray.addPatch(linearPath0, point);

    point = mesharray.getStop(SvgMeshPatch::Right, 0, 0).point;
    mesharray.addPatch(linearPath1, point);

    mesharray.newRow();
    point = mesharray.getStop(SvgMeshPatch::Bottom, 0, 0).point;
    mesharray.addPatch(linearPath2, point);

    point = mesharray.getStop(SvgMeshPatch::Bottom, 0, 1).point;
    mesharray.addPatch(linearPath3, point);

    QList<QPointF> path = {QPointF(50,50), QPointF(150,50)};
    QCOMPARE(mesharray.getPath(SvgMeshPatch::Top, 0, 0), path);

    // compare the common side Top Bottom, col = 0
    path = {QPointF(50,150), QPointF(150,150)};
    QCOMPARE(mesharray.getPath(SvgMeshPatch::Top, 1, 0), path);
    std::reverse(path.begin(), path.end());
    QCOMPARE(mesharray.getPath(SvgMeshPatch::Bottom, 0, 0), path);

    // compare the common side Top Bottom, col = 1
    path = {QPointF(150,150), QPointF(250,150)};
    QCOMPARE(mesharray.getPath(SvgMeshPatch::Top, 1, 1), path);
    std::reverse(path.begin(), path.end());
    QCOMPARE(mesharray.getPath(SvgMeshPatch::Bottom, 0, 1), path);

    // compare the common side Right-Left, row = 0
    path = {QPointF(150,150), QPointF(150,50)};
    QCOMPARE(mesharray.getPath(SvgMeshPatch::Left, 0, 1), path);
    path = {QPointF(150,150), QPointF(150,50)};
    std::reverse(path.begin(), path.end());
    QCOMPARE(mesharray.getPath(SvgMeshPatch::Right, 0, 0), path);

    // compare the common side Right-Left, row = 1
    path = {QPointF(150,150), QPointF(150,250)};
    QCOMPARE(mesharray.getPath(SvgMeshPatch::Right, 1, 0), path);
    std::reverse(path.begin(), path.end());
    QCOMPARE(mesharray.getPath(SvgMeshPatch::Left, 1, 1), path);

    path = {QPointF(50,250), QPointF(50,150)};
    QCOMPARE(mesharray.getPath(SvgMeshPatch::Left, 1, 0), path);

    path = {QPointF(250,250), QPointF(150,250)};
    QCOMPARE(mesharray.getPath(SvgMeshPatch::Bottom, 1, 1), path);

    path = {QPointF(250,150), QPointF(250,250)};
    QCOMPARE(mesharray.getPath(SvgMeshPatch::Right, 1, 1), path);
}

QTEST_MAIN(TestMeshArray)

