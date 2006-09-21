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

#ifndef KIS_WDG_NOISE_H
#define KIS_WDG_NOISE_H

#include <kis_filter_config_widget.h>

class Ui_WdgNoiseOptions;
class KisFilter;

class KisWdgNoise : public KisFilterConfigWidget
{
    Q_OBJECT
    public:
        KisWdgNoise(KisFilter* nfilter, QWidget* parent = 0, const char* name = 0);
        ~KisWdgNoise();
    public:
        inline Ui_WdgNoiseOptions* widget() { return m_widget; };
        virtual void setConfiguration(KisFilterConfiguration*);
    private:
        Ui_WdgNoiseOptions* m_widget;
};

#endif

