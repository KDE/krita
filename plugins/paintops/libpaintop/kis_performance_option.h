/* This file is part of the KDE project
 * Copyright (C) Scott Petrovic <scottpetrovic@gmail.com>, (C) 2016
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

#ifndef KIS_PERFORMANCE_H
#define KIS_PERFORMANCE_H

#include <brushengine/kis_paint_information.h>
#include <kritapaintop_export.h>
#include <kis_paintop_option.h>

/**
 * The global preferences option defines settings that happen across all brushes
 */
class PAINTOP_EXPORT KisPerformanceOption : public KisPaintOpOption
{
    Q_OBJECT

public:
    KisPerformanceOption(bool createConfigWidget = false);
    ~KisPerformanceOption();

private:
    bool m_createConfigWidget;
};




#endif
 
