/*
 * SPDX-FileCopyrightText: 2021 Mathias Wein <lynx.mw+kde@gmail.com>
 *  SPDX-FileCopyrightText: 2023 Srirupa Datta <srirupa.sps@gmail.com>
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
#include <KisResourceThumbnailPainter.h>
#include "wdg_resource_preview.h"


class KisActionManager;
class KisResourceTypeModel;
class KisStorageModel;
class KisTagModel;
class KisResourceModel;
class KisTagFilterResourceProxyModel;
class KisTag;
class KisWdgTagSelectionControllerOneResource;

namespace Ui
{
class WdgDlgResourceManager;
} // namespace Ui

class DlgResourceManager : public KoDialog
{
    Q_OBJECT
public:
    DlgResourceManager(KisActionManager* actionMgr, QWidget *parent = 0);
    ~DlgResourceManager() override;

private Q_SLOTS:
    void slotResourcesSelectionChanged(QModelIndex selected);

    void slotDeleteResources();
    void slotImportResources();
    void slotOpenResourceFolder();
    void slotCreateBundle();
    void slotSaveTags();
private:
    void updateDeleteButtonState(const QModelIndexList &list);

    static QString constructMetadata(const QMap<QString, QVariant> &metadata, const QString &resourceType);

private:
    QWidget *m_page {nullptr};
    QScopedPointer<Ui::WdgDlgResourceManager> m_ui;
    KisActionManager *m_actionManager {nullptr};

    QScopedPointer<KisWdgTagSelectionControllerOneResource> m_tagsController;

    KisResourceThumbnailPainter m_thumbnailPainter;

    bool m_undeleteMode {false};

    WdgResourcePreview *m_wdgResourcePreview;
};

#endif // DLGRESOURCEMANAGER_H
