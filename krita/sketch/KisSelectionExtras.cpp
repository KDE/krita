/* This file is part of the KDE project
 * Copyright (C) 2013 Camilla Boemann <cbo@boemann.dk>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "KisSelectionExtras.h"

#include <kis_debug.h>

#include <kis_selection_filters.h>
#include <operations/kis_filter_selection_operation.h>

#include "KisSketchView.h"

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
    KisSelectionFilter *filter = new KisBorderSelectionFilter(xradius, yradius);
    KisFilterSelectionOperation opr("border-oper");
    opr.runFilter(filter, m_view, KisOperationConfiguration());
}

void KisSelectionExtras::feather(qint32 radius)
{
    KisSelectionFilter *filter = new KisFeatherSelectionFilter(radius);
    KisFilterSelectionOperation opr("feather-oper");
    opr.runFilter(filter, m_view, KisOperationConfiguration());
}
