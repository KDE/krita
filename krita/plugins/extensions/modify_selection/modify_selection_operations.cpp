/*
 *  Copyright (c) 2012 Dmitry Kazakov <dimula73@gmail.com>
 *  Copyright (c) 2013 Sven Langkamp <sven.langkamp@gmail.com>
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

#include "modify_selection_operations.h"
#include <kis_selection_filters.h>

void GrowSelectionOperation::runFromXML(KisView2* view, const KisOperationConfiguration& config)
{
    int xradius = config.getInt("x-radius", 1);
    int yradius = config.getInt("y-radius", 1);
    KisSelectionFilter* filter = new KisGrowSelectionFilter(xradius, yradius);
    runFilter(filter, view, config);
}

void ShrinkSelectionOperation::runFromXML(KisView2* view, const KisOperationConfiguration& config)
{
    int xradius = config.getInt("x-radius", 1);
    int yradius = config.getInt("y-radius", 1);
    bool edgeLock = config.getBool("edgeLock", false);
    KisSelectionFilter* filter = new KisShrinkSelectionFilter(xradius, yradius, edgeLock);
    runFilter(filter, view, config);
}

void BorderSelectionOperation::runFromXML(KisView2* view, const KisOperationConfiguration& config)
{
    int xradius = config.getInt("x-radius", 1);
    int yradius = config.getInt("y-radius", 1);
    KisSelectionFilter* filter = new KisBorderSelectionFilter(xradius, yradius);
    runFilter(filter, view, config);
}

void FeatherSelectionOperation::runFromXML(KisView2* view, const KisOperationConfiguration& config)
{
    int radius = config.getInt("radius", 1);
    KisSelectionFilter* filter = new KisFeatherSelectionFilter(radius);
    runFilter(filter, view, config);
}

void SmoothSelectionOperation::runFromXML(KisView2* view, const KisOperationConfiguration& config)
{
    Q_UNUSED(config);
    KisSelectionFilter* filter = new KisSmoothSelectionFilter();
    runFilter(filter, view, config);
}

