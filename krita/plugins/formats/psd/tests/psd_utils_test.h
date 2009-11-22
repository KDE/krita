/*
 * Copyright (C) 2009 Boudewijn Rempt <boud@valdyas.org>
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

#ifndef _PSD_UTILS_TEST_H_
#define _PSD_UTILS_TEST_H_

#include <QObject>

class PSDUtilsTest : public QObject
{
    Q_OBJECT
private slots:

    void test_psdwrite_quint8();
    void test_psdwrite_quint16();
    void test_psdwrite_quint32();
    void test_psdwrite_qstring();
    void test_psdwrite_pascalstring();
    void test_psdpad();

    void test_psdread_quint8();
    void test_psdread_quint16();
    void test_psdread_quint32();
    void test_psdread_pascalstring();

};

#endif
