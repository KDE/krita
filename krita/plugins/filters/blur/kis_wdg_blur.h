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
 
#ifndef _KIS_WDG_BLUR_H_
#define _KIS_WDG_BLUR_H_

// TODO: remove that
#define LCMS_HEADER <lcms.h>
// TODO: remove it !

#include <kis_filter_config_widget.h>

class KisFilter;
class Ui_WdgBlur;

class KisWdgBlur : public KisFilterConfigWidget
{
    Q_OBJECT
    public:
        KisWdgBlur( KisFilter* nfilter, QWidget * parent, const char * name);
        inline Ui_WdgBlur* widget() { return m_widget; };
        virtual void setConfiguration(KisFilterConfiguration*);
    private slots:
        void linkSpacingToggled(bool);
        void spinBoxHalfWidthChanged(int );
        void spinBoxHalfHeightChanged(int );
    private:
        bool m_halfSizeLink;
        Ui_WdgBlur* m_widget;
};

#endif
