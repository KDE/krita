/*
 *  Copyright (c) 2004 Cyrille Berger <cberger@cberger.net>
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

#ifndef _KIS_MULTI_INTEGER_FILTER_WIDGET_H_
#define _KIS_MULTI_INTEGER_FILTER_WIDGET_H_

#include <vector>

#include <knuminput.h>
#include <kis_filter_config_widget.h>
#include "koffice_export.h"

class KIntNumInput;

struct KisIntegerWidgetParam {
    KRITA_EXPORT KisIntegerWidgetParam(  Q_INT32 nmin, Q_INT32 nmax, Q_INT32 ninitvalue, QString nname);
    Q_INT32 min;
    Q_INT32 max;
    Q_INT32 initvalue;
    QString name;
};

typedef std::vector<KisIntegerWidgetParam> vKisIntegerWidgetParam;

class KRITA_EXPORT KisMultiIntegerFilterWidget : public KisFilterConfigWidget
{
    Q_OBJECT
public:
    KisMultiIntegerFilterWidget(QWidget * parent,  const char * name, const char *caption, vKisIntegerWidgetParam iwparam);

    virtual void setConfiguration(KisFilterConfiguration * config);
    
public:
    inline Q_INT32 nbValues() { return m_nbintegerWidgets; };
    inline Q_INT32 valueAt( Q_INT32 i ) { return m_integerWidgets[i]->value(); };
    
private:
    KIntNumInput** m_integerWidgets;
    Q_INT32 m_nbintegerWidgets;
};

#endif
