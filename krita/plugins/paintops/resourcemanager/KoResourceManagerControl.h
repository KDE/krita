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

#ifndef KORESOURCEMANAGERCONTROL_H
#define KORESOURCEMANAGERCONTROL_H

#include "KoResourceBundle.h"
#include <QtCore/QModelIndex>

class KoXmlResourceBundleMeta;
class KoXmlResourceBundleManifest;
class KoResourceBundleManager;
class KoResourceTableModel;
template <class T> class KoResourceServer;

class KoResourceManagerControl
{

public:
    KoResourceManagerControl();
    ~KoResourceManagerControl();

    KoResourceTableModel* getModel();
    void launchServer();

    void about();
    void createPack();
    void filterResourceTypes(int index);
    void modifySelected(int mode);
    void refreshCurrentTable();
    bool rename(QModelIndex index,QString);
    void setMeta(QModelIndex index,QString,QString);

private:
    KoXmlResourceBundleMeta *meta;
    KoXmlResourceBundleManifest *manifest;
    KoResourceBundleManager *extractor;
    KoResourceServer<KoResourceBundle> *bundleServer;
    KoResourceTableModel *model;
    QString root;

    enum {
        Install=0,
        Uninstall,
        Delete
    };
};

#endif // KORESOURCEMANAGERCONTROL_H

