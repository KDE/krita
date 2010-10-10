/*
 *  This file is part of KOffice tests
 *
 *  Copyright (C) 2009-2010 Casper Boemann <casper.boemann@kogmbh.com>
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
#ifndef TESTKOTABLECOLUMNANDROWSTYLEMANAGER_H
#define TESTKOTABLECOLUMNANDROWSTYLEMANAGER_H

#include <QObject>
#include <QtTest/QtTest>

class TestKoTableColumnAndRowStyleManager : public QObject
{
    Q_OBJECT
public:
    TestKoTableColumnAndRowStyleManager() {}

private slots:
    // basic testing of the manager.
    void testManager();

};

#endif // TESTKOTABLECOLUMNANDROWSTYLEMANAGER_H
