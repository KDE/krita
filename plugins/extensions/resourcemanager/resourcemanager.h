/*
 * resourcemanager.h -- Part of Krita
 *
 * SPDX-FileCopyrightText: 2014 Boudewijn Rempt (boud@valdyas.org)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef RESOURCEMANAGER_H
#define RESOURCEMANAGER_H

#include <QVariant>
#include <QStringList>
#include <QString>

#include <KoResourceServer.h>

#include <KisActionPlugin.h>
#include <KoResourceBundle.h>

#include "dlg_create_bundle.h"

class ResourceManager : public KisActionPlugin
{
    Q_OBJECT
public:
    ResourceManager(QObject *parent, const QVariantList &);
    ~ResourceManager() override;
    KoResourceBundleSP saveBundle(const DlgCreateBundle &dlgCreateBundle);

private Q_SLOTS:
    void slotCreateBundle();
    void slotManageBundles();

    void slotImportBrushes();
    void slotImportGradients();
    void slotImportPalettes();
    void slotImportPatterns();
    void slotImportPresets();
    void slotImportWorkspaces();
    void slotImportBundles();


private:

    QStringList importResources(const QString &title, const QStringList &mimes) const;

    class Private;
    QScopedPointer<Private> d;
};

#endif // RESOURCEMANAGER_H
