/*
 *  Copyright (c) 2010 Dmitry Kazakov <dimula73@gmail.com>
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

#ifndef KIS_SIMPLE_UPDATE_QUEUE_TEST_H
#define KIS_SIMPLE_UPDATE_QUEUE_TEST_H

#include <QtTest/QtTest>


class KisSimpleUpdateQueueTest : public QObject
{
    Q_OBJECT

private:
    void testSplit(bool useFullRefresh);

private slots:
    void testJobProcessing();
    void testSplitUpdate();
    void testSplitFullRefresh();
    void testChecksum();
    void testMixingTypes();
};

#endif /* KIS_SIMPLE_UPDATE_QUEUE_TEST_H */

