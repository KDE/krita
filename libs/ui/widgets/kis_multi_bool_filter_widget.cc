/*
 *  SPDX-FileCopyrightText: 2005 Michael Thaler <michael.thaler@physik.tu-muenchen.de>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "widgets/kis_multi_bool_filter_widget.h"
#include <QLabel>
#include <QLayout>
#include <QCheckBox>
#include <QVBoxLayout>
#include <filter/kis_filter_configuration.h>
#include <KisGlobalResourcesInterface.h>

#include <klocalizedstring.h>

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
    widgetLayout->setContentsMargins(0,0,0,0);

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


void KisMultiBoolFilterWidget::setConfiguration(const KisPropertiesConfigurationSP  config)
{
    if (!config) return;
    for (int i = 0; i < nbValues(); ++i) {
        bool val = config->getBool(m_boolWidgets[i]->objectName(), true);
        m_boolWidgets[i]->setChecked(val);
    }
}


KisPropertiesConfigurationSP KisMultiBoolFilterWidget::configuration() const
{
    KisFilterConfigurationSP config = new KisFilterConfiguration(m_filterid, 0, KisGlobalResourcesInterface::instance());
    for (int i = 0; i < nbValues(); ++i) {
        config->setProperty(m_boolWidgets[i]->objectName(), valueAt(i));
    }
    return config;
}

