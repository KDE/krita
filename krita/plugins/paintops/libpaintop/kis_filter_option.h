/* This file is part of the KDE project
 * Copyright (C) Boudewijn Rempt <boud@valdyas.org>, (C) 2008
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

#ifndef KIS_FILTER_OPTION_H
#define KIS_FILTER_OPTION_H

#include "kis_paintop_option.h"
#include <kis_types.h>

class KisFilterConfiguration;
/**
 * The filter option allows the user to select a particular filter
 * that can be applied by the paintop to the brush footprint or the
 * original paint device data.
 */
class KisFilterOption : public KisPaintOpOption
{

public:

    KisFilterOption();

    /**
     * Return the currently selected filter
     */
    const KisFilterSP filter() const;

    /**
     * Return the currently selected filter configuration
     */
    KisFilterConfiguration* filterConfig() const;


};

#endif
