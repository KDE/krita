/*
 *  SPDX-FileCopyrightText: 2009 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_FILTER_SELECTIONS_BENCHMARK_H
#define KIS_FILTER_SELECTIONS_BENCHMARK_H

#include <simpletest.h>
#include "kis_selection.h"
#include "filter/kis_filter.h"
#include "filter/kis_filter_configuration.h"
#include "filter/kis_filter_registry.h"
#include "kis_processing_information.h"


class KisFilterSelectionsBenchmark : public QObject
{
    Q_OBJECT
private Q_SLOTS:

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
    KisFilterConfigurationSP m_configuration;
};

#endif
