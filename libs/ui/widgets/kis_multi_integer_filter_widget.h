/*
 *  SPDX-FileCopyrightText: 2004 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef _KIS_MULTI_INTEGER_FILTER_WIDGET_H_
#define _KIS_MULTI_INTEGER_FILTER_WIDGET_H_

#include <kis_config_widget.h>
#include "kritaui_export.h"
#include <kis_debug.h>
#include <QVector>
#include <QSpinBox>
#include "kis_slider_spin_box.h"
#include "kis_int_parse_spin_box.h"

#include <vector>

class KisDelayedActionIntegerInput : public KisSliderSpinBox
{
    Q_OBJECT

public:
    KisDelayedActionIntegerInput(QWidget * parent, const QString & name);
    void cancelDelayedSignal();

private Q_SLOTS:
    void slotValueChanged();
    void slotTimeToUpdate();

Q_SIGNALS:
    void valueChangedDelayed(int value);

private:
    QTimer * m_timer;
};


struct KRITAUI_EXPORT KisIntegerWidgetParam {

    KisIntegerWidgetParam(qint32 nmin, qint32 nmax, qint32 ninitvalue, const QString& label, const QString& nname);

    qint32 min;
    qint32 max;
    qint32 initvalue;
    QString label;
    QString name;
};

typedef std::vector<KisIntegerWidgetParam> vKisIntegerWidgetParam;

class KRITAUI_EXPORT KisMultiIntegerFilterWidget : public KisConfigWidget
{
    Q_OBJECT
public:
    KisMultiIntegerFilterWidget(const QString& filterid, QWidget* parent, const QString& caption, vKisIntegerWidgetParam iwparam);
    ~KisMultiIntegerFilterWidget() override;

    void setConfiguration(const KisPropertiesConfigurationSP config) override;
    KisPropertiesConfigurationSP configuration() const override;

private:

    qint32 nbValues() const;
    qint32 valueAt(qint32 i);

    QVector<KisDelayedActionIntegerInput*> m_integerWidgets;
    QString m_filterid;
    KisPropertiesConfigurationSP m_config;
};

#endif
