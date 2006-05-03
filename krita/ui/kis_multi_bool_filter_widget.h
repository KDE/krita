/*
 *  Copyright (c) 2005 Michael Thaler <michael.thaler@physik.tu-muenchen.de>
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

#ifndef _KIS_MULTI_BOOL_FILTER_WIDGET_H_
#define _KIS_MULTI_BOOL_FILTER_WIDGET_H_

#include <vector>

#include <qcheckbox.h>

#include "koffice_export.h"
#include <kis_filter_config_widget.h>

class KIntNumInput;

struct KisBoolWidgetParam {
    KRITA_EXPORT KisBoolWidgetParam(  bool ninitvalue, QString label, QString name);
    bool initvalue;
    QString label;
    QString name;

};

typedef std::vector<KisBoolWidgetParam> vKisBoolWidgetParam;

class KRITA_EXPORT KisMultiBoolFilterWidget : public KisFilterConfigWidget
{
    Q_OBJECT
public:
    KisMultiBoolFilterWidget(QWidget * parent,  const char * name, QString caption, vKisBoolWidgetParam iwparam);
    virtual void setConfiguration(KisFilterConfiguration * cfg);
public:
    inline qint32 nbValues() { return m_nbboolWidgets; };
    inline qint32 valueAt( qint32 i ) { return m_boolWidgets[i]->isChecked(); };
private:
    QCheckBox** m_boolWidgets;
    qint32 m_nbboolWidgets;
};

#endif
