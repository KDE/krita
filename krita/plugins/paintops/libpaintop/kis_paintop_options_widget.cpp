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
    QMap<QListWidgetItem*, KisPaintOpOption*> checkBoxMap;
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
            
    connect(m_d->optionsList, SIGNAL(itemChanged(QListWidgetItem*)), SLOT(itemChanged(QListWidgetItem*)));
}


KisPaintOpOptionsWidget::~KisPaintOpOptionsWidget()
{
    qDeleteAll(m_d->paintOpOptions);
    delete m_d;
}

void KisPaintOpOptionsWidget::addPaintOpOption(KisPaintOpOption * option)
{
    if (!option->configurationPage()) return;

    m_d->paintOpOptions << option;

    connect(option, SIGNAL(sigSettingChanged()), SIGNAL(sigConfigurationItemChanged()));

    m_d->optionsStack->addWidget(option->configurationPage());
        
    QListWidgetItem * item = new QListWidgetItem(m_d->optionsList);
    item->setText(option->label());
    Qt::ItemFlags flags = item->flags();
    flags = Qt::ItemIsSelectable | Qt::ItemIsEnabled;
    if (option->isCheckable()) {

        flags |= Qt::ItemIsUserCheckable;
        item->setCheckState((option->isChecked()) ? Qt::Checked : Qt::Unchecked);
        m_d->checkBoxMap[item] = option;
    }
    item->setFlags(flags);
}

void KisPaintOpOptionsWidget::setConfiguration(const KisPropertiesConfiguration * config)
{
    Q_ASSERT(!config->getString("paintop").isEmpty());
    foreach(KisPaintOpOption* option, m_d->paintOpOptions) {
        option->readOptionSetting(config);
    }
    foreach(QListWidgetItem* checkableItem, m_d->checkBoxMap.keys()) {
        checkableItem->setCheckState((m_d->checkBoxMap[checkableItem]->isChecked()) ? Qt::Checked : Qt::Unchecked);
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

void KisPaintOpOptionsWidget::changePage(QListWidgetItem *current, QListWidgetItem *previous)
{
    Q_UNUSED(previous);
    m_d->optionsStack->setCurrentIndex(m_d->optionsList->row(current));
}

void KisPaintOpOptionsWidget::itemChanged(QListWidgetItem* item)
{
    if(m_d->checkBoxMap.contains(item)) {
        m_d->checkBoxMap[item]->setChecked(item->checkState() == Qt::Checked);
        emit sigConfigurationItemChanged();
    }
}

#include "kis_paintop_options_widget.moc"
