/*
 *  Copyright (c) 2010 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
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

#ifndef _KO_COLOR_SPACES_BENCHMARK_H_
#define _KO_COLOR_SPACES_BENCHMARK_H_

#include <QtTest/QtTest>

class KoColorSpacesBenchmark : public QObject
{
    Q_OBJECT
private:
    void createRowsColumns();
private slots:
    void benchmarkAlpha_data();
    void benchmarkAlpha();
    void benchmarkAlpha2_data();
    void benchmarkAlpha2();
    void benchmarkSetAlpha_data();
    void benchmarkSetAlpha();
    void benchmarkSetAlpha2_data();
    void benchmarkSetAlpha2();
    void benchmarkSetAlphaIndividualCall_data();
    void benchmarkSetAlphaIndividualCall();
    void benchmarkSetAlpha2IndividualCall_data();
    void benchmarkSetAlpha2IndividualCall();
};

#endif
