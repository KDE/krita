/*
 *  Copyright (c) 2014 Victor Lafon metabolic.ewilan@hotmail.fr
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
#include "dlg_bundle_manager.h"
#include "ui_wdgdlgbundlemanager.h"

#include "resourcemanager.h"

#include <QListWidget>
#include <QListWidgetItem>
#include <QPainter>

#include <KoIcon.h>

#define ICON_SIZE 48

DlgBundleManager::DlgBundleManager(QWidget *parent)
    : KDialog(parent)
    , m_page(new QWidget())
    , m_ui(new Ui::WdgDlgBundleManager)

{
    setCaption(i18n("Manage Resource Bundles"));
    m_ui->setupUi(m_page);
    setMainWidget(m_page);
    resize(m_page->sizeHint());
    setButtons(Ok | Cancel);
    setDefaultButton(Ok);

    m_ui->bnAdd->setIcon(koIcon("arrow-right"));
    connect(m_ui->bnAdd, SIGNAL(clicked()), SLOT(addSelected()));

    m_ui->bnRemove->setIcon(koIcon("arrow-left"));
    connect(m_ui->bnRemove, SIGNAL(clicked()), SLOT(removeSelected()));

    KoResourceServer<KoResourceBundle> *bundleServer = ResourceBundleServerProvider::instance()->resourceBundleServer();


    foreach(const QString &f, bundleServer->blackListedFiles()) {
        KoResourceBundle *bundle = new KoResourceBundle(f);
        bundle->load();
        if (bundle->valid()) {
            m_blacklistedBundles[f] = bundle;
        }
    }
    fillListWidget(m_blacklistedBundles.values(), m_ui->listInactive);

    foreach(KoResourceBundle *bundle, bundleServer->resources()) {
        qDebug() << "\tbunde" << bundle->name() << bundle->valid();
        if (bundle->valid()) {
            m_activeBundles[bundle->filename()] = bundle;
        }
    }
    fillListWidget(m_activeBundles.values(), m_ui->listActive);
}

void DlgBundleManager::addSelected()
{

}

void DlgBundleManager::removeSelected()
{

}

void DlgBundleManager::fillListWidget(QList<KoResourceBundle *> bundles, QListWidget *w)
{
    w->setIconSize(QSize(ICON_SIZE, ICON_SIZE));
    w->setSelectionMode(QAbstractItemView::MultiSelection);

    foreach(KoResourceBundle *bundle, bundles) {
        QPixmap pixmap(ICON_SIZE, ICON_SIZE);
        if (!bundle->image().isNull()) {
            QImage scaled = bundle->image().scaled(ICON_SIZE, ICON_SIZE, Qt::KeepAspectRatio, Qt::SmoothTransformation);
            int x = (ICON_SIZE - scaled.width()) / 2;
            int y = (ICON_SIZE - scaled.height()) / 2;
            QPainter gc(&pixmap);
            gc.drawImage(x, y, scaled);
            gc.end();
        }
        else {
            pixmap.fill(Qt::gray);
        }

        QListWidgetItem *item = new QListWidgetItem(pixmap, bundle->name());
        item->setData(Qt::UserRole, bundle->filename());
        w->addItem(item);
    }
}

