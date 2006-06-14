/*
 * This file is part of Krita
 *
 * Copyright (c) 2006 Cyrille Berger <cberger@cberger.net>
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

#ifndef KIS_WDG_RANDOMPICK_H
#define KIS_WDG_RANDOMPICK_H

#include <kis_filter_config_widget.h>

class WdgRandomPickOptions;
class KisFilter;

class KisWdgRandomPick : public KisFilterConfigWidget
{
    Q_OBJECT
    public:
        KisWdgRandomPick(KisFilter* nfilter, QWidget* parent = 0, const char* name = 0);
        ~KisWdgRandomPick();
    public:
        inline WdgRandomPickOptions* widget() { return m_widget; };
        virtual void setConfiguration(KisFilterConfiguration*);
    private:
        WdgRandomPickOptions* m_widget;
};

#endif

