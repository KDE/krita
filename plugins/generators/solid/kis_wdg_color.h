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

#ifndef KIS_WDG_COLOR_H
#define KIS_WDG_COLOR_H

#include <kis_config_widget.h>

class Ui_WdgColorOptions;

class KisWdgColor : public KisConfigWidget
{
    Q_OBJECT
public:
    KisWdgColor(QWidget* parent = 0);
    ~KisWdgColor();
public:
    inline const Ui_WdgColorOptions* widget() const {
        return m_widget;
    }
    virtual void setConfiguration(const KisPropertiesConfiguration*);
    virtual KisPropertiesConfiguration* configuration() const;
private:
    Ui_WdgColorOptions* m_widget;
};

#endif

