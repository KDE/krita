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
    KoResourceManagerControl(int nb);
    ~KoResourceManagerControl();

    KoResourceTableModel* getModel(int type);
    void launchServer();

    int getNbModels();

    void about();
    void createPack(int type);
    void filterResourceTypes(int index);
    void modifySelected(int mode,int type);
    bool rename(QModelIndex index,QString,int type);
    void setMeta(QModelIndex index,QString metaType,QString metaValue, int type);
    void saveMeta(QModelIndex index,int type);

private:
    KoXmlResourceBundleMeta *meta;
    KoXmlResourceBundleManifest *manifest;
    KoResourceBundleManager *extractor;
    KoResourceServer<KoResourceBundle> *bundleServer;
    QList<KoResourceTableModel*> modelList;
    QString root;
    int nbModels;

    enum {
        Install=0,
        Uninstall,
        Delete
    };
};

#endif // KORESOURCEMANAGERCONTROL_H

