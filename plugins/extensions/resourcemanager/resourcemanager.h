/*
 * resourcemanager.h -- Part of Krita
 *
 * Copyright (c) 2014 Boudewijn Rempt (boud@valdyas.org)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#ifndef RESOURCEMANAGER_H
#define RESOURCEMANAGER_H

#include <QVariant>
#include <QStringList>
#include <QString>

#include <KoResourceServer.h>

#include <KisActionPlugin.h>
#include "KisResourceBundle.h"

#include "dlg_create_bundle.h"

class ResourceManager : public KisActionPlugin
{
    Q_OBJECT
public:
    ResourceManager(QObject *parent, const QVariantList &);
    ~ResourceManager() override;
    KisResourceBundleSP saveBundle(const DlgCreateBundle &dlgCreateBundle);

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
