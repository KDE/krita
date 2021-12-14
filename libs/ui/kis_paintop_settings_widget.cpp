/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2008 Boudewijn Rempt <boud@valdyas.org>
 * SPDX-FileCopyrightText: 2011 Silvio Heinrich <plassy@web.de>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
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

#include <brushengine/kis_paintop_preset.h>
#include <kis_cmb_composite.h>
#include <kis_categorized_item_delegate.h>
#include <brushengine/kis_locked_properties_server.h>
#include <brushengine/kis_locked_properties_proxy.h>
#include <brushengine/kis_locked_properties.h>
#include <brushengine/kis_paintop_lod_limitations.h>



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
    m_d->optionsList = new KisCategorizedListView(this);
    m_d->optionsList->setModel(m_d->model);
    m_d->optionsList->setItemDelegate(new KisCategorizedItemDelegate(m_d->optionsList));
    m_d->optionsList->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
    m_d->optionsList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    QSizePolicy policy =  QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    m_d->optionsList->setSizePolicy(policy);

    m_d->optionsList->setMinimumWidth(160); // this should be just big enough to show all of the setting names

    m_d->optionsStack = new QStackedWidget(this);
    policy = QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_d->optionsStack->setSizePolicy(policy);

    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->addWidget(m_d->optionsList);
    layout->addWidget(m_d->optionsStack);

    layout->setStretch(0, 0);
    layout->setStretch(1, 1);

    m_saveLockedOption = false;

    connect(m_d->optionsList, SIGNAL(activated(QModelIndex)), this, SLOT(changePage(QModelIndex)));
    connect(m_d->optionsList, SIGNAL(clicked(QModelIndex)), this, SLOT(changePage(QModelIndex)));
    connect(m_d->optionsList, SIGNAL(rightClickedMenuDropSettingsTriggered()), this, SLOT(slotLockPropertiesDrop()));
    connect(m_d->optionsList, SIGNAL(rightClickedMenuSaveSettingsTriggered()), this, SLOT(slotLockPropertiesSave()));
    connect(m_d->optionsList, SIGNAL(sigEntryChecked(QModelIndex)), this, SLOT(slotEntryChecked(QModelIndex)));
    connect (m_d->optionsList, SIGNAL(lockAreaTriggered(QModelIndex)), this, SLOT(lockProperties(QModelIndex)));

}


KisPaintOpSettingsWidget::~KisPaintOpSettingsWidget()
{
    qDeleteAll(m_d->paintOpOptions);
    delete m_d;
}

void KisPaintOpSettingsWidget::addPaintOpOption(KisPaintOpOption *option)
{
    addPaintOpOption(option, option->category());
}

void KisPaintOpSettingsWidget::addPaintOpOption(KisPaintOpOption *option, KisPaintOpOption::PaintopCategory category)
{
    if (!option->configurationPage()) return;
    m_d->model->addPaintOpOption(option, m_d->optionsStack->count(), option->label(), category);
    connect(option, SIGNAL(sigSettingChanged()), SIGNAL(sigConfigurationItemChanged()));
    m_d->optionsStack->addWidget(option->configurationPage());
    m_d->paintOpOptions << option;
}

void KisPaintOpSettingsWidget::addPaintOpOption(KisPaintOpOption *option, QString category)
{
    if (!option->configurationPage()) return;
    m_d->model->addPaintOpOption(option, m_d->optionsStack->count(), option->label(), category);
    connect(option, SIGNAL(sigSettingChanged()), SIGNAL(sigConfigurationItemChanged()));
    m_d->optionsStack->addWidget(option->configurationPage());
    m_d->paintOpOptions << option;
}

void KisPaintOpSettingsWidget::setConfiguration(const KisPropertiesConfigurationSP  config)
{
    Q_ASSERT(!config->getString("paintop").isEmpty());
    KisLockedPropertiesProxySP propertiesProxy = KisLockedPropertiesServer::instance()->createLockedPropertiesProxy(config);
    int indexcount = 0;
    Q_FOREACH (KisPaintOpOption* option, m_d->paintOpOptions) {
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
}

void KisPaintOpSettingsWidget::writeConfiguration(KisPropertiesConfigurationSP config) const
{
    KisLockedPropertiesProxySP propertiesProxy = KisLockedPropertiesServer::instance()->createLockedPropertiesProxy(config);
    Q_FOREACH (const KisPaintOpOption* option, m_d->paintOpOptions) {
        option->startWriteOptionSetting(propertiesProxy);
    }
}

KisPaintopLodLimitations KisPaintOpSettingsWidget::lodLimitations() const
{
    KisPaintopLodLimitations l;

    Q_FOREACH (const KisPaintOpOption* option, m_d->paintOpOptions) {
        if (option->isCheckable() && !option->isChecked()) continue;
        option->lodLimitations(&l);
    }

    return l;
}

void KisPaintOpSettingsWidget::setImage(KisImageWSP image)
{
    KisPaintOpConfigWidget::setImage(image);

    Q_FOREACH (KisPaintOpOption* option, m_d->paintOpOptions) {
        option->setImage(image);
    }
}

void KisPaintOpSettingsWidget::setNode(KisNodeWSP node)
{
    KisPaintOpConfigWidget::setNode(node);

    Q_FOREACH (KisPaintOpOption* option, m_d->paintOpOptions) {
        option->setNode(node);
    }
}

void KisPaintOpSettingsWidget::setResourcesInterface(KisResourcesInterfaceSP resourcesInterface)
{
    KisPaintOpConfigWidget::setResourcesInterface(resourcesInterface);

    Q_FOREACH (KisPaintOpOption* option, m_d->paintOpOptions) {
        option->setResourcesInterface(resourcesInterface);
    }
}

void KisPaintOpSettingsWidget::setCanvasResourcesInterface(KoCanvasResourcesInterfaceSP canvasResourcesInterface)
{
    KisPaintOpConfigWidget::setCanvasResourcesInterface(canvasResourcesInterface);

    Q_FOREACH (KisPaintOpOption* option, m_d->paintOpOptions) {
        option->setCanvasResourcesInterface(canvasResourcesInterface);
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

        // disable the widget if a setting area is not active and not being used
       if (info.option->isCheckable() ) {
            m_d->optionsStack->setEnabled(info.option->isChecked());
       } else {
           m_d->optionsStack->setEnabled(true); // option is not checkable, so always enable
       }


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
        KisPropertiesConfigurationSP p = new KisPropertiesConfiguration();
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
