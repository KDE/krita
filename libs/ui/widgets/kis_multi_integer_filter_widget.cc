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

#include "widgets/kis_multi_integer_filter_widget.h"
#include <QLabel>
#include <QLayout>
#include <QTimer>
#include <QSpinBox>
#include <QGridLayout>

#include <filter/kis_filter_configuration.h>
#include <klocalizedstring.h>

KisDelayedActionIntegerInput::KisDelayedActionIntegerInput(QWidget * parent, const QString & name)
    : KisIntParseSpinBox(parent)
{
    setObjectName(name);
    m_timer = new QTimer(this);
    m_timer->setObjectName(name);
    m_timer->setSingleShot(true);
    connect(m_timer, SIGNAL(timeout()), SLOT(slotValueChanged()));
    connect(this, SIGNAL(valueChanged(int)), SLOT(slotTimeToUpdate()));
}

void KisDelayedActionIntegerInput::slotTimeToUpdate()
{
    m_timer->start(50);
}

void KisDelayedActionIntegerInput::slotValueChanged()
{
    emit valueChangedDelayed(value());
}

void KisDelayedActionIntegerInput::cancelDelayedSignal()
{
    m_timer->stop();
}

KisIntegerWidgetParam::KisIntegerWidgetParam(qint32 nmin, qint32 nmax, qint32 ninitvalue, const QString & label, const QString & nname)
    : min(nmin)
    , max(nmax)
    , initvalue(ninitvalue)
    , label(label)
    , name(nname)
{
}

KisMultiIntegerFilterWidget::KisMultiIntegerFilterWidget(const QString& filterid,
                                                         QWidget* parent,
                                                         const QString& caption,
                                                         vKisIntegerWidgetParam iwparam)
    : KisConfigWidget(parent)
    , m_filterid(filterid)
    , m_config(new KisFilterConfiguration(filterid, 0))
{
    this->setWindowTitle(caption);

    QGridLayout *widgetLayout = new QGridLayout(this);
    widgetLayout->setColumnStretch(1, 1);
    widgetLayout->setContentsMargins(0,0,0,0);
    widgetLayout->setHorizontalSpacing(0);

    for (uint i = 0; i < iwparam.size(); ++i) {
        KisDelayedActionIntegerInput *widget = new KisDelayedActionIntegerInput(this, iwparam[i].name);

        widget->setRange(iwparam[i].min, iwparam[i].max);
        widget->setValue(iwparam[i].initvalue);
        widget->cancelDelayedSignal();

        connect(widget, SIGNAL(valueChangedDelayed(int)), SIGNAL(sigConfigurationItemChanged()));

        QLabel* lbl = new QLabel(iwparam[i].label + ':', this);
        widgetLayout->addWidget(lbl, i , 0);

        widgetLayout->addWidget(widget, i , 1);

        m_integerWidgets.append(widget);
    }
    widgetLayout->setRowStretch(iwparam.size(),1);

    QSpacerItem * sp = new QSpacerItem(1, 1);
    widgetLayout->addItem(sp, iwparam.size(), 0);
}

KisMultiIntegerFilterWidget::~KisMultiIntegerFilterWidget()
{
}

void KisMultiIntegerFilterWidget::setConfiguration(const KisPropertiesConfigurationSP config)
{
    if (!config) return;

    if (!m_config) {
        m_config = new KisFilterConfiguration(m_filterid, 0);
    }

    m_config->fromXML(config->toXML());
    for (int i = 0; i < nbValues(); ++i) {
        KisDelayedActionIntegerInput*  w = m_integerWidgets[i];
        if (w) {
            int val = config->getInt(m_integerWidgets[i]->objectName());
            m_integerWidgets[i]->setValue(val);
            m_integerWidgets[i]->cancelDelayedSignal();
        }
    }
}

KisPropertiesConfigurationSP KisMultiIntegerFilterWidget::configuration() const
{
    KisFilterConfigurationSP config = new KisFilterConfiguration(m_filterid, 0);
    if (m_config) {
        config->fromXML(m_config->toXML());
    }

    for (int i = 0; i < nbValues(); ++i) {
        KisDelayedActionIntegerInput*  w = m_integerWidgets[i];
        if (w) {
            config->setProperty(w->objectName(), w->value());
        }
    }
    return config;
}

qint32 KisMultiIntegerFilterWidget::nbValues() const {
    return m_integerWidgets.size();
}

qint32 KisMultiIntegerFilterWidget::valueAt(qint32 i) {
    if (i < m_integerWidgets.size()) {
        return m_integerWidgets[i]->value();
    }
    else {
        warnKrita << "Trying to access integer widget" << i << "but there are only" << m_integerWidgets.size() << "widgets";
        return 0;
    }
}


