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
#ifndef TESTMESHARRAY_H
#define TESTMESHARRAY_H

#include <QObject>
#include <QColor>

/*
 * This is a rudimentary test to test if the path form by
 * by meshpatch in mesharray are correct. This can be dropped
 * later for the superior rendering based test.
 */
class TestMeshArray : public QObject
{
    Q_OBJECT
public:
    TestMeshArray();

private Q_SLOTS:
    void testSinglePatch();
    void test_2_by_2_Patch();

private:
    const QList<QPair<QString, QColor>> patch0 = {
        { "c  25,-25  75, 25  100,0" ,  QColor("#000000") },
        { "c  25, 25 -25, 75  0,100" ,  QColor("#800080") },
        { "c -25, 25 -75,-25 -100,0" ,  QColor("#ff0000") },
        { "c -25,-25, 25,-75"        ,  QColor("#800080") },
    };
    const QList<QPair<QString, QColor>> patch1 = {
        { "c  25,-25  75, 25  100,0" ,  QColor("#000000") },
        { "c  25, 25 -25, 75  0,100" ,  QColor("#000000") },
        { "c -25, 25 -75,-25"        ,  QColor("#800080") },
    };
    const QList<QPair<QString, QColor>> patch2 = {
        { "c  25, 25 -25, 75  0,100" ,  QColor("#000000") },
        { "c -25, 25 -75,-25 -100,0" ,  QColor("#ffff00") },
        { "c -25,-25, 25,-75"        ,  QColor("#008000") },
    };
    const QList<QPair<QString, QColor>> patch3 = {
        { "c  25, 25 -25, 75  0,100" ,  QColor("#000000") },
        { "c -25, 25 -75,-25"        ,  QColor("#000000") },
    };
};

#endif  // TESTMESHARRAY_H
