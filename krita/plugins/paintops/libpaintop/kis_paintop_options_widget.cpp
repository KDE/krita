/* This file is part of the KDE project
 * Copyright (C) Boudewijn Rempt <boud@valdyas.org>, (C) 2008
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "kis_paintop_options_widget.h"
#include "kis_paintop_option.h"

#include <QList>
#include <QLabel>
#include <QMap>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QListWidget>
#include <QCheckBox>
#include <QStackedWidget>
#include <klocale.h>
#include <kis_paintop_preset.h>

class KisPaintOpOptionsWidget::Private
{

public:

    QList<KisPaintOpOption*> paintOpOptions;
    QMap<QListWidgetItem*, KisPaintOpOption*> widgetOptionMap;
    QListWidget * optionsList;
    QStackedWidget * optionsStack;
};

KisPaintOpOptionsWidget::KisPaintOpOptionsWidget(QWidget * parent)
        : KisPaintOpSettingsWidget(parent)
        , m_d(new Private())
{
    setObjectName("KisPaintOpPresetsWidget");
    QHBoxLayout * layout = new QHBoxLayout(this);
    m_d->optionsList = new QListWidget(this);
    m_d->optionsList->setFixedWidth(128);
    QSizePolicy policy =  QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    policy.setHorizontalStretch(0);
    m_d->optionsList->setSizePolicy(policy);

    m_d->optionsStack = new QStackedWidget(this);
    policy = QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    policy.setHorizontalStretch(3);
    m_d->optionsStack->setSizePolicy(policy);
    layout->addWidget(m_d->optionsList);
    layout->addWidget(m_d->optionsStack);

    connect(m_d->optionsList,
            SIGNAL(currentItemChanged(QListWidgetItem *, QListWidgetItem *)),
            this, SLOT(changePage(QListWidgetItem *, QListWidgetItem*)));
}


KisPaintOpOptionsWidget::~KisPaintOpOptionsWidget()
{
    delete m_d;
}

void KisPaintOpOptionsWidget::addPaintOpOption(KisPaintOpOption * option)
{
    if (!option->configurationPage()) return;

    m_d->paintOpOptions << option;

    connect(option, SIGNAL(sigSettingChanged()), SIGNAL(sigConfigurationItemChanged()));

    if (option->isCheckable()) {
        QWidget* w = new QWidget;
        QVBoxLayout* l = new QVBoxLayout;
        QCheckBox* c = new QCheckBox(i18n("Active"));
        c->setChecked(option->isChecked());
        connect(c, SIGNAL(toggled(bool)), option, SLOT(setChecked(bool)));
        l->addWidget(c);
        l->addWidget(option->configurationPage());
        //option->configurationPage()->setVisible( true );
        l->addSpacing(1);
        w->setLayout(l);

        m_d->optionsStack->addWidget(w);
    } else {
        m_d->optionsStack->addWidget(option->configurationPage());
    }

    QListWidgetItem * item = new QListWidgetItem(m_d->optionsList);
    m_d->widgetOptionMap[item] = option;
    item->setText(option->label());
    Qt::ItemFlags flags = item->flags();
    if (option->isCheckable()) {

        flags |= Qt::ItemIsUserCheckable;
    }
    item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
}

void KisPaintOpOptionsWidget::setConfiguration(const KisPropertiesConfiguration * config)
{
    Q_ASSERT(!config->getString("paintop").isEmpty());
    foreach(KisPaintOpOption* option, m_d->paintOpOptions) {
        option->readOptionSetting(config);
    }
}

void KisPaintOpOptionsWidget::writeConfiguration(KisPropertiesConfiguration *config) const
{
    foreach(const KisPaintOpOption* option, m_d->paintOpOptions) {
        option->writeOptionSetting(config);
    }
}

void KisPaintOpOptionsWidget::changePage(QListWidgetItem *current, QListWidgetItem *previous)
{
    Q_UNUSED(previous);
    m_d->optionsStack->setCurrentIndex(m_d->optionsList->row(current));
}


#include "kis_paintop_options_widget.moc"
