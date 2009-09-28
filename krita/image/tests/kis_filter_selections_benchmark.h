/*
 *  Copyright (c) 2009 Dmitry  Kazakov <dimula73@gmail.com>
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

#ifndef KIS_FILTER_SELECTIONS_BENCHMARK_H
#define KIS_FILTER_SELECTIONS_BENCHMARK_H

#include <QtTest/QtTest>
#include "kis_selection.h"
#include "filter/kis_filter.h"
#include "filter/kis_filter_configuration.h"
#include "filter/kis_filter_registry.h"
#include "kis_processing_information.h"


class KisFilterSelectionsBenchmark : public QObject
{
    Q_OBJECT
private slots:

    void testAll();

private:
    void initSelection();
    void initFilter(const QString &name);
    void testFilter(const QString &name);
    void testUsualSelections(int num);
    void testNoSelections(int num);
    void testGoodSelections(int num);
    void testBitBltWOSelections(int num);
    void testBitBltSelections(int num);
private:
    KisSelectionSP m_selection;
    KisPaintDeviceSP m_device;
    KisFilterSP m_filter;
    KisFilterConfiguration *m_configuration;
};

#endif
