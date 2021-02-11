/*
 * SPDX-FileCopyrightText: 2021 Mathias Wein <lynx.mw+kde@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef DLGRESOURCEMANAGER_H
#define DLGRESOURCEMANAGER_H

#include <KoDialog.h>
#include <QScopedPointer>
#include <QSortFilterProxyModel>
#include <QSharedPointer>
#include <QItemSelection>

class KisResourceTypeModel;
class KisStorageModel;
class KisTagModel;
class KisResourceModel;
class KisTagFilterResourceProxyModel;
class KisTag;

namespace Ui
{
class WdgDlgResourceManager;
}

class DlgResourceManager : public KoDialog
{
    Q_OBJECT
public:
    DlgResourceManager(QWidget *parent = 0);
    ~DlgResourceManager() override;




private Q_SLOTS:
    void slotResourceTypeSelected(int);
    void slotStorageSelected(int);
    void slotTagSelected(int);

    void slotResourcesSelectionChanged(QModelIndex selected);

    void slotImportResources();
    void slotOpenResourceFolder();
    void slotImportBundle();
    void slotDeleteBackupFiles();
private:
    QString getCurrentResourceType();
    int getCurrentStorageId();
    QSharedPointer<KisTag> getCurrentTag();


private:
    QWidget *m_page;
    QScopedPointer<Ui::WdgDlgResourceManager> m_ui;
    KisResourceTypeModel *m_resourceTypeModel {0};
    KisStorageModel *m_storageModel {0};
    KisTagModel *m_tagModel {0};
    QMap<QString, KisTagModel*> m_tagModelsForResourceType;

    KisResourceModel *m_resourceModel;
    QMap<QString, KisResourceModel*> m_resourceModelsForResourceType;
    QMap<QString, KisTagFilterResourceProxyModel*> m_resourceProxyModelsForResourceType;

};

#endif // DLGRESOURCEMANAGER_H
