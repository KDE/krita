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
    void slotManageBundles();
    void slotManageResources();

private:
    class Private;
    QScopedPointer<Private> d;
};

#endif // RESOURCEMANAGER_H
