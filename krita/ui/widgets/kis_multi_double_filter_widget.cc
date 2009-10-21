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

#include "widgets/kis_multi_double_filter_widget.h"
#include <QLabel>
#include <QLayout>
#include <QTimer>
#include <QGridLayout>

#include <knuminput.h>
#include <kis_config_widget.h>
#include <filter/kis_filter_configuration.h>
#include <klocale.h>

KisDelayedActionDoubleInput::KisDelayedActionDoubleInput(QWidget * parent, const QString & name)
        : KDoubleNumInput(parent)
{
    setObjectName(name);
    m_timer = new QTimer(this);
    m_timer->setObjectName(name);
    m_timer->setSingleShot(true);
    connect(m_timer, SIGNAL(timeout()), SLOT(slotValueChanged()));
    connect(this, SIGNAL(valueChanged(double)), SLOT(slotTimeToUpdate()));
}

void KisDelayedActionDoubleInput::slotTimeToUpdate()
{
    m_timer->start(50);
}

void KisDelayedActionDoubleInput::slotValueChanged()
{
    emit valueChangedDelayed(value());
}

void KisDelayedActionDoubleInput::cancelDelayedSignal()
{
    m_timer->stop();
}

KisDoubleWidgetParam::KisDoubleWidgetParam(double nmin, double nmax, double ninitvalue, const QString & nlabel, const QString & nname) :
        min(nmin),
        max(nmax),
        initvalue(ninitvalue),
        label(nlabel),
        name(nname)
{

}

KisMultiDoubleFilterWidget::KisMultiDoubleFilterWidget(const QString & filterid, QWidget * parent, const QString & caption, vKisDoubleWidgetParam dwparam)
        : KisConfigWidget(parent), m_filterid(filterid)
{
    m_nbdoubleWidgets = dwparam.size();

    this->setWindowTitle(caption);

    QGridLayout *widgetLayout = new QGridLayout(this);
    widgetLayout->setColumnStretch(1, 1);

    m_doubleWidgets = new KisDelayedActionDoubleInput*[ m_nbdoubleWidgets ];

    for (qint32 i = 0; i < m_nbdoubleWidgets; ++i) {
        m_doubleWidgets[i] = new KisDelayedActionDoubleInput(this, dwparam[i].name);
        m_doubleWidgets[i]->setRange(dwparam[i].min, dwparam[i].max);
        m_doubleWidgets[i]->setValue(dwparam[i].initvalue);
        m_doubleWidgets[i]->cancelDelayedSignal();

        connect(m_doubleWidgets[i], SIGNAL(valueChangedDelayed(double)), SIGNAL(sigConfigurationItemChanged()));

        QLabel* lbl = new QLabel(dwparam[i].label + ':', this);
        widgetLayout->addWidget(lbl, i , 0);

        widgetLayout->addWidget(m_doubleWidgets[i], i , 1);
    }
    QSpacerItem * sp = new QSpacerItem(1, 1);
    widgetLayout->addItem(sp, m_nbdoubleWidgets, 0);

}

void KisMultiDoubleFilterWidget::setConfiguration(const KisPropertiesConfiguration * config)
{
    if (!config) return;

    for (int i = 0; i < m_nbdoubleWidgets ; ++i) {
        KisDelayedActionDoubleInput *  w = m_doubleWidgets[i];
        if (w) {
            double val = config->getDouble(m_doubleWidgets[i]->objectName());
            m_doubleWidgets[i]->setValue(val);
            m_doubleWidgets[i]->cancelDelayedSignal();
        }
    }
}

KisPropertiesConfiguration* KisMultiDoubleFilterWidget::configuration() const
{
    KisFilterConfiguration* config = new KisFilterConfiguration(m_filterid, 0);
    for (int i = 0; i < nbValues(); ++i) {
        config->setProperty(m_doubleWidgets[i]->objectName(), m_doubleWidgets[i]->value());
    }
    return config;
}

#include "kis_multi_double_filter_widget.moc"
