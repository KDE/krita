/*
 * OpenRPT report writer and rendering engine
 * Copyright (C) 2012 Dag Andersen <danders@get2net.dk>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef KOREPORTTEST_H
#define KOREPORTTEST_H

#include <QtTest>


class KoReportTest : public QObject
{
    Q_OBJECT
private slots:
    void pageOptions();
    void lineItem();
    void rectItem();
};

#endif // KOREPORTTEST_H
