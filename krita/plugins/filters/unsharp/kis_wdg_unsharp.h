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

#ifndef _KIS_WDG_UNSHARP_H_
#define _KIS_WDG_UNSHARP_H_

#include <kis_config_widget.h>

class KisFilter;
class Ui_WdgUnsharp;

class KisWdgUnsharp : public KisConfigWidget
{
    Q_OBJECT
public:
    KisWdgUnsharp(QWidget * parent);
    inline const Ui_WdgUnsharp* widget() const {
        return m_widget;
    }
    virtual void setConfiguration(const KisPropertiesConfiguration*);
    virtual KisPropertiesConfiguration* configuration() const;
private:
    Ui_WdgUnsharp* m_widget;
};

#endif
