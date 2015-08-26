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

#include "kis_paintop_settings_widget.h"
#include "kis_paintop_option.h"
#include "kis_paintop_options_model.h"

#include <QHBoxLayout>
#include <QList>
#include <QStackedWidget>
#include <QVBoxLayout>
#include <QPainter>
#include <QStyleOptionViewItem>
#include <QMenu>
#include <QAction>
#include <QShowEvent>

#include <kis_paintop_preset.h>
#include <kis_cmb_composite.h>
#include <kis_categorized_item_delegate.h>
#include <kis_locked_properties_server.h>
#include <kis_locked_properties_proxy.h>
#include <kis_locked_properties.h>
#include <kis_paintop_lod_limitations.h>



struct KisPaintOpSettingsWidget::Private
{
    QList<KisPaintOpOption*>    paintOpOptions;
    KisCategorizedListView*     optionsList;
    KisPaintOpOptionListModel*  model;
    QStackedWidget*             optionsStack;

};

KisPaintOpSettingsWidget::KisPaintOpSettingsWidget(QWidget * parent)
        : KisPaintOpConfigWidget(parent)
        , m_d(new Private())
{
    setObjectName("KisPaintOpPresetsWidget");
    
    m_d->model       = new KisPaintOpOptionListModel(this);
    m_d->optionsList = new KisCategorizedListView(false, this);
    m_d->optionsList->setModel(m_d->model);
    m_d->optionsList->setItemDelegate(new KisCategorizedItemDelegate(false, m_d->optionsList));
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

    m_saveLockedOption = false;

    connect(m_d->optionsList, SIGNAL(activated(const QModelIndex&)), this, SLOT(changePage(const QModelIndex&)));
    connect(m_d->optionsList, SIGNAL(clicked(QModelIndex)), this, SLOT(changePage(const QModelIndex&)));
    connect(m_d->optionsList, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(lockProperties(const QModelIndex&)));
    connect(m_d->optionsList, SIGNAL(rightClickedMenuDropSettingsTriggered()), this, SLOT(slotLockPropertiesDrop()));
    connect(m_d->optionsList, SIGNAL(rightClickedMenuSaveSettingsTriggered()), this, SLOT(slotLockPropertiesSave()));
    connect(m_d->optionsList, SIGNAL(sigEntryChecked(QModelIndex)), this, SLOT(slotEntryChecked(QModelIndex)));

}


KisPaintOpSettingsWidget::~KisPaintOpSettingsWidget()
{
    qDeleteAll(m_d->paintOpOptions);
    delete m_d;
}

void KisPaintOpSettingsWidget::addPaintOpOption(KisPaintOpOption *option, const QString &label)
{
    if (!option->configurationPage()) return;
    m_d->model->addPaintOpOption(option, m_d->optionsStack->count(), label);
    connect(option, SIGNAL(sigSettingChanged()), SIGNAL(sigConfigurationItemChanged()));
    m_d->optionsStack->addWidget(option->configurationPage());
    m_d->paintOpOptions << option;

}

void KisPaintOpSettingsWidget::setConfiguration(const KisPropertiesConfiguration * config)
{
    Q_ASSERT(!config->getString("paintop").isEmpty());
    KisLockedPropertiesProxy* propertiesProxy = KisLockedPropertiesServer::instance()->createLockedPropertiesProxy(config);
    int indexcount = 0;
    foreach (KisPaintOpOption* option, m_d->paintOpOptions) {
        option->startReadOptionSetting(propertiesProxy);

        if (KisLockedPropertiesServer::instance()->propertiesFromLocked()) {
            option->setLocked(true);
        }
        else {
            option->setLocked(false);
        }

        KisLockedPropertiesServer::instance()->setPropertiesFromLocked(false);
        KisOptionInfo info;
        info.option = option;
        info.index = indexcount;
        m_d->model->categoriesMapper()->itemFromRow(m_d->model->indexOf(info).row())->setLocked(option->isLocked());
        m_d->model->categoriesMapper()->itemFromRow(m_d->model->indexOf(info).row())->setLockable(true);
        m_d->model->signalDataChanged(m_d->model->indexOf(info));
        indexcount++;
    }

    KisPaintOpConfigWidget::setConfiguration(propertiesProxy);
    delete propertiesProxy;
}

void KisPaintOpSettingsWidget::writeConfiguration(KisPropertiesConfiguration *config) const
{
    KisLockedPropertiesProxy* propertiesProxy = KisLockedPropertiesServer::instance()->createLockedPropertiesProxy(config);
    foreach(const KisPaintOpOption* option, m_d->paintOpOptions) {
        option->startWriteOptionSetting(propertiesProxy);
    }

    KisPaintOpConfigWidget::writeConfiguration(propertiesProxy);
    delete propertiesProxy;
}

KisPaintopLodLimitations KisPaintOpSettingsWidget::lodLimitations() const
{
    KisPaintopLodLimitations l;

    foreach(const KisPaintOpOption* option, m_d->paintOpOptions) {
        if (option->isCheckable() && !option->isChecked()) continue;
        option->lodLimitations(&l);
    }

    return l;
}

void KisPaintOpSettingsWidget::setImage(KisImageWSP image)
{
    foreach(KisPaintOpOption* option, m_d->paintOpOptions) {
        option->setImage(image);
    }
}

void KisPaintOpSettingsWidget::setNode(KisNodeWSP node)
{
    foreach(KisPaintOpOption* option, m_d->paintOpOptions) {
        option->setNode(node);
    }
}

void KisPaintOpSettingsWidget::changePage(const QModelIndex& index)
{
    KisOptionInfo info;
    QPalette palette;
    palette.setColor(QPalette::Base, QColor(255,200,200));
    palette.setColor(QPalette::Text, Qt::black);
    
    if(m_d->model->entryAt(info, index)) {
        m_d->optionsStack->setCurrentIndex(info.index);
    }

    notifyPageChanged();
}

void KisPaintOpSettingsWidget::notifyPageChanged()
{
}

void KisPaintOpSettingsWidget::lockProperties(const QModelIndex& index)
{
    KisOptionInfo info;
    if (m_d->model->entryAt(info, index)) {
        m_d->optionsList->setCurrentIndex(index);
        KisPropertiesConfiguration* p = new KisPropertiesConfiguration();
        info.option->startWriteOptionSetting(p);

        if (!info.option->isLocked()){
            KisLockedPropertiesServer::instance()->addToLockedProperties(p);
            info.option->setLocked(true);
            m_d->model->categoriesMapper()->itemFromRow(index.row())->setLocked(true);
        }
        else {
            KisLockedPropertiesServer::instance()->removeFromLockedProperties(p);
            info.option->setLocked(false);
            m_d->model->categoriesMapper()->itemFromRow(index.row())->setLocked(false);
            if (m_saveLockedOption){
                emit sigSaveLockedConfig(p);
            }
            else {
                emit sigDropLockedConfig(p);
            }
            m_saveLockedOption = false;
        }
        m_d->model->signalDataChanged(index);
    }

}
void KisPaintOpSettingsWidget::slotLockPropertiesDrop()
{
    m_saveLockedOption = false;
    lockProperties(m_d->optionsList->currentIndex());
}
void KisPaintOpSettingsWidget::slotLockPropertiesSave()
{
    m_saveLockedOption = true;
    lockProperties(m_d->optionsList->currentIndex());
}
void KisPaintOpSettingsWidget::slotEntryChecked(const QModelIndex &index)
{
    Q_UNUSED(index);
    emit sigConfigurationItemChanged();
}



#include "kis_paintop_settings_widget.moc"
