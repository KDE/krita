/*
 *  Copyright (c) 2019 Anna Medonosova <anna.medonosova@gmail.com>
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

#ifndef KISAPPIMAGEUPDATERTEST_H
#define KISAPPIMAGEUPDATERTEST_H

#include <QObject>

class KisAppimageUpdaterTest : public QObject
{
    Q_OBJECT
public:
    explicit KisAppimageUpdaterTest(QObject *parent = nullptr);

private Q_SLOTS:
    void testCheckForUpdate();
    void testCheckForUpdate_data();
    void testDoUpdate();
    void testDoUpdate_data();
};

#endif // KISAPPIMAGEUPDATERTEST_H
