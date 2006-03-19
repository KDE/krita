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

#include "kis_multi_integer_filter_widget.h"

#include <qlabel.h>
#include <qlayout.h>
#include <qtimer.h>

#include <knuminput.h>
#include <klocale.h>

KisDelayedActionIntegerInput::KisDelayedActionIntegerInput(QWidget * parent, const char * name)
        : KIntNumInput(parent, name)
{
        m_timer = new QTimer(this, name);
        connect(m_timer, SIGNAL(timeout()), SLOT(slotValueChanged()));
        connect(this, SIGNAL(valueChanged( int )), SLOT(slotTimeToUpdate()));
}

void KisDelayedActionIntegerInput::slotTimeToUpdate()
{
    m_timer->start(50, true);
}

void KisDelayedActionIntegerInput::slotValueChanged()
{
    emit valueChangedDelayed( value() );
}

void KisDelayedActionIntegerInput::cancelDelayedSignal()
{
    m_timer->stop();
}

KisIntegerWidgetParam::KisIntegerWidgetParam(  Q_INT32 nmin, Q_INT32 nmax, Q_INT32 ninitvalue, QString label, QString nname) :
    min(nmin),
    max(nmax),
    initvalue(ninitvalue),
    label(label),
    name(nname)
{
}

KisMultiIntegerFilterWidget::KisMultiIntegerFilterWidget(QWidget * parent,
                                                         const char * name,
                                                         const char * caption,
                                                         vKisIntegerWidgetParam iwparam)
    : KisFilterConfigWidget( parent, name )
{
    m_nbintegerWidgets = iwparam.size();
    this->setCaption(caption);

    QGridLayout *widgetLayout = new QGridLayout(this, m_nbintegerWidgets + 1, 3);
    widgetLayout->setColStretch ( 1, 1 );

    m_integerWidgets = new KisDelayedActionIntegerInput*[ m_nbintegerWidgets ];

    for( Q_INT32 i = 0; i < m_nbintegerWidgets; ++i)
    {
        m_integerWidgets[i] = new KisDelayedActionIntegerInput( this, iwparam[i].name.ascii());
        m_integerWidgets[i]->setRange( iwparam[i].min, iwparam[i].max);
        m_integerWidgets[i]->setValue( iwparam[i].initvalue );
        m_integerWidgets[i]->cancelDelayedSignal();

        connect(m_integerWidgets[i], SIGNAL(valueChangedDelayed( int )), SIGNAL(sigPleaseUpdatePreview()));

        QLabel* lbl = new QLabel(iwparam[i].label+":", this);
        widgetLayout->addWidget( lbl, i , 0);

        widgetLayout->addWidget( m_integerWidgets[i], i , 1);
    }
    QSpacerItem * sp = new QSpacerItem(1, 1);
    widgetLayout->addItem(sp, m_nbintegerWidgets, 0);
}

void KisMultiIntegerFilterWidget::setConfiguration( KisFilterConfiguration * config )
{
    for (int i = 0; i < nbValues(); ++i) {
        KisDelayedActionIntegerInput *  w = m_integerWidgets[i];
        if (w) {
            int val = config->getInt(m_integerWidgets[i]->name());
            m_integerWidgets[i]->setValue(val);
            m_integerWidgets[i]->cancelDelayedSignal();
        }
    }
}

#include "kis_multi_integer_filter_widget.moc"
