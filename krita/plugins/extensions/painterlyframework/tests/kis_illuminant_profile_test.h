/*
 *  Copyright (c) 2007 Emanuele Tamponi <emanuele@valinor.it>
 *
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

#ifndef KIS_ILLUMINANT_PROFILE_TEST_H_
#define KIS_ILLUMINANT_PROFILE_TEST_H_

#include <QtTest/QtTest>
#include <QStringList>

class KisIlluminantProfileTest : public QObject
{
    Q_OBJECT

private slots:

    void initTestCase();
    void testLoading();
    void testSaving();

    // TODO: void testFromToRgb();

private:

    QStringList list;

};

#endif // KIS_ILLUMINANT_PROFILE_TEST_H_
