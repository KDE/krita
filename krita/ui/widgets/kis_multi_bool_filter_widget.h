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

#include <QCheckBox>
#include <QVector>

#include "krita_export.h"
#include <kis_config_widget.h>

class KisPropertiesConfiguration;

struct KisBoolWidgetParam {
    KRITAUI_EXPORT KisBoolWidgetParam(bool ninitvalue, const QString & label, const QString & name);
    bool initvalue;
    QString label;
    QString name;

};

typedef std::vector<KisBoolWidgetParam> vKisBoolWidgetParam;

class KRITAUI_EXPORT KisMultiBoolFilterWidget : public KisConfigWidget
{
    Q_OBJECT

public:

    KisMultiBoolFilterWidget(const QString & filterid, QWidget * parent,  const QString & caption, vKisBoolWidgetParam iwparam);

    virtual void setConfiguration(const KisPropertiesConfiguration* cfg);

    virtual KisPropertiesConfiguration* configuration() const;

public:

    inline qint32 nbValues() const {
        return m_boolWidgets.count();
    }

    inline bool valueAt(qint32 i) const {
        return m_boolWidgets[i]->isChecked();
    }

private:

    QVector<QCheckBox*> m_boolWidgets;
    QString m_filterid;
};

#endif
