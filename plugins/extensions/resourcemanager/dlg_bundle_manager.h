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
#ifndef DLG_BUNDLE_MANAGER_H
#define DLG_BUNDLE_MANAGER_H

#include <QWidget>

#include <KoDialog.h>
#include "kis_action_manager.h"
#include "resourcemanager.h"

class KisResourceBundle;
class QListWidget;
class QListWidgetItem;

namespace Ui
{
class WdgDlgBundleManager;
}

class DlgBundleManager : public KoDialog
{
    Q_OBJECT
public:
    explicit DlgBundleManager(ResourceManager *resourceManager, KisActionManager* actionMgr, QWidget *parent = 0);

private Q_SLOTS:

    void accept() override;
    void addSelected();
    void removeSelected();
    void itemSelected(QListWidgetItem *current, QListWidgetItem *previous);
    void itemSelected(QListWidgetItem *current);
    void editBundle();
    void slotImportResource();
    void slotCreateBundle();
    void slotOpenResourceFolder();

private:

    QWidget *m_page;
    Ui::WdgDlgBundleManager *m_ui;

    void fillListWidget(QList<KisResourceBundleSP> bundles, QListWidget *w);
    void refreshListData();

    QMap<QString, KisResourceBundleSP> m_blacklistedBundles;
    QMap<QString, KisResourceBundleSP> m_activeBundles;

    KisResourceBundleSP m_currentBundle;
    KisActionManager *m_actionManager;
    ResourceManager *m_resourceManager;
};

#endif // DLG_BUNDLE_MANAGER_H
