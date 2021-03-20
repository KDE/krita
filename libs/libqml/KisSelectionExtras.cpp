/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2013 Camilla Boemann <cbo@boemann.dk>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "KisSelectionExtras.h"

#include <kis_debug.h>

#include <kis_selection_filters.h>
#include <operations/kis_filter_selection_operation.h>

KisSelectionExtras::KisSelectionExtras(KisViewManager *view)
    : m_view(view)
{
}

KisSelectionExtras::~KisSelectionExtras()
{
}

void KisSelectionExtras::grow(qint32 xradius, qint32 yradius)
{
    KisSelectionFilter *filter = new KisGrowSelectionFilter(xradius, yradius);
    KisFilterSelectionOperation opr("grow-oper");
    opr.runFilter(filter, m_view, KisOperationConfiguration());
}

void KisSelectionExtras::shrink(qint32 xradius, qint32 yradius, bool edge_lock)
{
    KisSelectionFilter *filter = new KisShrinkSelectionFilter(xradius, yradius, edge_lock);
    KisFilterSelectionOperation opr("shrink-oper");
    opr.runFilter(filter, m_view, KisOperationConfiguration());
}

void KisSelectionExtras::border(qint32 xradius, qint32 yradius)
{
    KisSelectionFilter *filter = new KisBorderSelectionFilter(xradius, yradius, true);
    KisFilterSelectionOperation opr("border-oper");
    opr.runFilter(filter, m_view, KisOperationConfiguration());
}

void KisSelectionExtras::feather(qint32 radius)
{
    KisSelectionFilter *filter = new KisFeatherSelectionFilter(radius);
    KisFilterSelectionOperation opr("feather-oper");
    opr.runFilter(filter, m_view, KisOperationConfiguration());
}
