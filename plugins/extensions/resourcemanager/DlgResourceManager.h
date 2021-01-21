/*
 * SPDX-FileCopyrightText: 2021 Mathias Wein <lynx.mw+kde@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef DLGRESOURCEMANAGER_H
#define DLGRESOURCEMANAGER_H

#include <KoDialog.h>
#include <QScopedPointer>

class KisResourceTypeModel;

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
    void slotImportResources();
    void slotOpenResourceFolder();
    void slotImportBundle();
    void slotDeleteBackupFiles();
private:
    QWidget *m_page;
    QScopedPointer<Ui::WdgDlgResourceManager> m_ui;
    KisResourceTypeModel *m_resourceTypeModel {0};
};

#endif // DLGRESOURCEMANAGER_H
