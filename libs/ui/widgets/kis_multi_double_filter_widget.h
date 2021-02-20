/*
 *  SPDX-FileCopyrightText: 2004 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef _KIS_MULTI_DOUBLE_FILTER_WIDGET_H_
#define _KIS_MULTI_DOUBLE_FILTER_WIDGET_H_

#include <vector>

#include <QDoubleSpinBox>

#include <kis_config_widget.h>
#include "kritaui_export.h"
#include "kis_slider_spin_box.h"
#include "kis_double_parse_spin_box.h"

class KisDelayedActionDoubleInput : public KisDoubleSliderSpinBox
{
    Q_OBJECT

public:
    KisDelayedActionDoubleInput(QWidget * parent, const QString & name);

    void cancelDelayedSignal();

private Q_SLOTS:
    void slotValueChanged();
    void slotTimeToUpdate();

Q_SIGNALS:
    void valueChangedDelayed(double value);

private:

    QTimer * m_timer;
};


struct KRITAUI_EXPORT KisDoubleWidgetParam {
    KisDoubleWidgetParam(double nmin, double nmax, double ninitvalue, const QString & label, const QString & nname);
    double min;
    double max;
    double initvalue;
    QString label;
    QString name;
};

typedef std::vector<KisDoubleWidgetParam> vKisDoubleWidgetParam;

class KRITAUI_EXPORT KisMultiDoubleFilterWidget : public KisConfigWidget
{
    Q_OBJECT
public:
    KisMultiDoubleFilterWidget(const QString & filterid, QWidget * parent, const QString & caption, vKisDoubleWidgetParam dwparam);
    void setConfiguration(const KisPropertiesConfigurationSP  cfg) override;
    KisPropertiesConfigurationSP configuration() const override;
public:
    inline qint32 nbValues() const {
        return m_nbdoubleWidgets;
    }
    inline double valueAt(qint32 i) {
        return m_doubleWidgets[i]->value();
    }
private:
    QVector<KisDelayedActionDoubleInput*> m_doubleWidgets;
    qint32 m_nbdoubleWidgets;
    QString m_filterid;
};

#endif
