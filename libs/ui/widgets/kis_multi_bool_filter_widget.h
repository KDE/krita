/*
 *  SPDX-FileCopyrightText: 2005 Michael Thaler <michael.thaler@physik.tu-muenchen.de>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef _KIS_MULTI_BOOL_FILTER_WIDGET_H_
#define _KIS_MULTI_BOOL_FILTER_WIDGET_H_

#include <vector>

#include <QCheckBox>
#include <QVector>

#include "kritaui_export.h"
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

    void setConfiguration(const KisPropertiesConfigurationSP cfg) override;

    KisPropertiesConfigurationSP configuration() const override;

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
