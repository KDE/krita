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

#ifndef MANAGERCONTROL_H
#define MANAGERCONTROL_H

#include <QString>

class QIODevice;
class KoXmlResourceBundleMeta;
class KoXmlResourceBundleManifest;

class ManagerControl
{
    KoXmlResourceBundleMeta *meta;
    KoXmlResourceBundleManifest *manifest;
    //KoResourceBundleManager *extractor;
    QString currentMeta;
    QString currentManifest;

public:
    ManagerControl();
    ~ManagerControl();
    void setMeta(QString,QString,QString);
    QIODevice* getDevice(QString);
    void installPack(QString);
    void uninstallPack(QString);
    void deletePack(QString);
    void refreshCurrentTable();
    void rename(QString,QString);
    void about();
};

#endif // MANAGERCONTROL_H

