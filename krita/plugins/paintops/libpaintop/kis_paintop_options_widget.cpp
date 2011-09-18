/* This file is part of the KDE project
 * Copyright (C) Boudewijn Rempt <boud@valdyas.org>, (C) 2008
 * Copyright (C) Silvio Heinrich <plassy@web.de>   , (C) 2011
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
#include "kis_paintop_options_model.h"

#include <QHBoxLayout>
#include <QList>
#include <QStackedWidget>
#include <QVBoxLayout>

#include <kis_paintop_preset.h>
#include <kis_cmb_composite.h>
#include <kis_categorized_item_delegate.h>

struct KisPaintOpOptionsWidget::Private
{
    QList<KisPaintOpOption*>    paintOpOptions;
    KisCategorizedListView*     optionsList;
    KisPaintOpOptionListModel*  model;
    KisCategorizedItemDelegate* delegate;
    QStackedWidget*             optionsStack;
};

KisPaintOpOptionsWidget::KisPaintOpOptionsWidget(QWidget * parent)
        : KisPaintOpSettingsWidget(parent)
        , m_d(new Private())
{
    setObjectName("KisPaintOpPresetsWidget");
    
    m_d->model       = new KisPaintOpOptionListModel();
    m_d->delegate    = new KisCategorizedItemDelegate(false);
    m_d->optionsList = new KisCategorizedListView();
    m_d->optionsList->setModel(m_d->model);
    m_d->optionsList->setItemDelegate(m_d->delegate);
    m_d->optionsList->setFixedWidth(128);
    
    QSizePolicy policy =  QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    policy.setHorizontalStretch(0);
    m_d->optionsList->setSizePolicy(policy);

    m_d->optionsStack = new QStackedWidget(this);
    policy = QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    policy.setHorizontalStretch(3);
    m_d->optionsStack->setSizePolicy(policy);
    
    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->addWidget(m_d->optionsList);
    layout->addWidget(m_d->optionsStack);

    connect(m_d->optionsList, SIGNAL(activated(const QModelIndex&)), this, SLOT(changePage(const QModelIndex&)));
    connect(m_d->optionsList, SIGNAL(clicked(QModelIndex))         , this, SLOT(changePage(const QModelIndex&)));
}


KisPaintOpOptionsWidget::~KisPaintOpOptionsWidget()
{
    qDeleteAll(m_d->paintOpOptions);
    delete m_d->model;
    delete m_d->delegate;
    delete m_d;
}

void KisPaintOpOptionsWidget::addPaintOpOption(KisPaintOpOption * option)
{
    if (!option->configurationPage()) return;
    
    m_d->model->addPaintOpOption(option, m_d->optionsStack->count());

    connect(option, SIGNAL(sigSettingChanged()), SIGNAL(sigConfigurationItemChanged()));

    m_d->optionsStack->addWidget(option->configurationPage());
    m_d->paintOpOptions << option;
}

void KisPaintOpOptionsWidget::setConfiguration(const KisPropertiesConfiguration * config)
{
    Q_ASSERT(!config->getString("paintop").isEmpty());
    
    m_d->model->reset();
    
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

void KisPaintOpOptionsWidget::setImage(KisImageWSP image)
{
    foreach(KisPaintOpOption* option, m_d->paintOpOptions) {
        option->setImage(image);
    }
}

void KisPaintOpOptionsWidget::changePage(const QModelIndex& index)
{
    KisOptionInfo info;
    
    if(m_d->model->entryAt(info, index.row())) {
        m_d->optionsStack->setCurrentIndex(info.index);
        emit sigConfigurationItemChanged();
    }
}

#include "kis_paintop_options_widget.moc"
