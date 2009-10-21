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

#include "widgets/kis_multi_bool_filter_widget.h"
#include <QLabel>
#include <QLayout>
#include <QCheckBox>
#include <QVBoxLayout>
#include <filter/kis_filter_configuration.h>

#include <klocale.h>

KisBoolWidgetParam::KisBoolWidgetParam(bool ninitvalue, const QString & nlabel, const QString & nname) :
        initvalue(ninitvalue),
        label(nlabel),
        name(nname)
{

}

KisMultiBoolFilterWidget::KisMultiBoolFilterWidget(const QString & filterid, QWidget * parent, const QString & caption, vKisBoolWidgetParam iwparam)
        : KisConfigWidget(parent)
        , m_filterid(filterid)
{
    qint32 nbboolWidgets = iwparam.size();

    this->setWindowTitle(caption);

    QVBoxLayout *widgetLayout = new QVBoxLayout(this);
    widgetLayout->setMargin(nbboolWidgets + 1);


    for (qint32 i = 0; i < nbboolWidgets; ++i) {
        QCheckBox * cb = new QCheckBox(this);
        cb->setObjectName(iwparam[i].name);
        cb->setChecked(iwparam[i].initvalue);
        cb->setText(iwparam[i].label);
        connect(cb, SIGNAL(toggled(bool)), SIGNAL(sigConfigurationItemChanged()));
        widgetLayout->addWidget(cb);
        m_boolWidgets.append(cb);
    }
    widgetLayout->addSpacerItem(new QSpacerItem(10, 10, QSizePolicy::Minimum, QSizePolicy::Expanding));
    widgetLayout->addStretch();
}


void KisMultiBoolFilterWidget::setConfiguration(const KisPropertiesConfiguration * config)
{
    if (!config) return;
    for (int i = 0; i < nbValues(); ++i) {
        double val = config->getBool(m_boolWidgets[i]->objectName());
        m_boolWidgets[i]->setChecked(val);
    }
}


KisPropertiesConfiguration* KisMultiBoolFilterWidget::configuration() const
{
    KisFilterConfiguration* config = new KisFilterConfiguration(m_filterid, 0);
    for (int i = 0; i < nbValues(); ++i) {
        config->setProperty(m_boolWidgets[i]->objectName(), valueAt(i));
    }
    return config;
}

#include "kis_multi_bool_filter_widget.moc"
