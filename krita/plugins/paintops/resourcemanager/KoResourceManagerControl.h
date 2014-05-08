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
#include <QModelIndex>
#include "resourcemanager.h"

class KoXmlResourceBundleMeta;
class KoXmlResourceBundleManifest;
class KoResourceBundleManager;
class KoResourceTableModel;

class KoResourceManagerControl : public QObject
{
    Q_OBJECT

public:
    explicit KoResourceManagerControl(int nb);
    ~KoResourceManagerControl();

    KoResourceTableModel* getModel(int type);
    int getNbModels();

    void addFiles(QString, int type);

    void filterResourceTypes(int index);


    bool createPack(int type);
    bool install(int type);
    bool uninstall(int type);
    bool remove(int type);

    void configureFilters(int filterType, bool enable);

    bool rename(QModelIndex index, QString, int type);
    void setMeta(QModelIndex index, QString metaType, QString metaValue, int type);
    void saveMeta(QModelIndex index, int type);
    void thumbnail(QModelIndex index, QString fileName, int type);
    void exportBundle(int type);
    bool importBundle();
    void refreshTaggingManager();

signals:
    void status(QString text, int timeout = 0);

private slots:
    void toStatus(QString text, int timeout = 0);

private:
    KoXmlResourceBundleMeta *m_meta;
    KoXmlResourceBundleManifest *m_manifest;
    KoResourceBundleManager *m_extractor;
    QList<KoResourceTableModel*> m_modelList;
    QString m_root;
    int m_modelsCount;

    enum {
        Install = 0,
        Uninstall,
        Delete
    };
};

#endif // KORESOURCEMANAGERCONTROL_H

