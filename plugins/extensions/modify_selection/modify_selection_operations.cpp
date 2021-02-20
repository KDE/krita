/*
 *  SPDX-FileCopyrightText: 2012 Dmitry Kazakov <dimula73@gmail.com>
 *  SPDX-FileCopyrightText: 2013 Sven Langkamp <sven.langkamp@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "modify_selection_operations.h"
#include <kis_selection_filters.h>

void GrowSelectionOperation::runFromXML(KisViewManager* view, const KisOperationConfiguration& config)
{
    int xradius = config.getInt("x-radius", 1);
    int yradius = config.getInt("y-radius", 1);
    KisSelectionFilter* filter = new KisGrowSelectionFilter(xradius, yradius);
    runFilter(filter, view, config);
}

void ShrinkSelectionOperation::runFromXML(KisViewManager* view, const KisOperationConfiguration& config)
{
    int xradius = config.getInt("x-radius", 1);
    int yradius = config.getInt("y-radius", 1);
    bool edgeLock = config.getBool("edgeLock", false);
    KisSelectionFilter* filter = new KisShrinkSelectionFilter(xradius, yradius, edgeLock);
    runFilter(filter, view, config);
}

void BorderSelectionOperation::runFromXML(KisViewManager* view, const KisOperationConfiguration& config)
{
    int xradius = config.getInt("x-radius", 1);
    int yradius = config.getInt("y-radius", 1);
    bool antialiasing = config.getInt("antialiasing", false);
    KisSelectionFilter* filter = new KisBorderSelectionFilter(xradius, yradius, antialiasing);
    runFilter(filter, view, config);
}

void FeatherSelectionOperation::runFromXML(KisViewManager* view, const KisOperationConfiguration& config)
{
    int radius = config.getInt("radius", 1);
    KisSelectionFilter* filter = new KisFeatherSelectionFilter(radius);
    runFilter(filter, view, config);
}

void SmoothSelectionOperation::runFromXML(KisViewManager* view, const KisOperationConfiguration& config)
{
    Q_UNUSED(config);
    KisSelectionFilter* filter = new KisSmoothSelectionFilter();
    runFilter(filter, view, config);
}

