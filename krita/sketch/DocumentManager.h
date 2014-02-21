/* This file is part of the KDE project
 * Copyright (C) 2012 Arjen Hiemstra <ahiemstra@heimr.nl>
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

#ifndef DOCUMENTMANAGER_H
#define DOCUMENTMANAGER_H

#include <QObject>

#include "image/krita_export.h"

class RecentFileManager;
class Settings;
class ProgressProxy;
class KisDoc2;
class KisSketchPart;
class KRITASKETCH_EXPORT DocumentManager : public QObject
{
    Q_OBJECT
public:
    KisDoc2* document() const;
    KisSketchPart* part();
    ProgressProxy* progressProxy() const;
    Settings* settingsManager() const;
    void setSettingsManager(Settings* newManager);
    RecentFileManager* recentFileManager() const;

public Q_SLOTS:
    void newDocument(int width, int height, float resolution);
    void openDocument(const QString& document, bool import = false);
    void closeDocument();
    bool save();
    void saveAs(const QString &filename, const QString &mimetype);
    void reload();

    static DocumentManager* instance();

Q_SIGNALS:
    void documentChanged();
    void aboutToDeleteDocument();
    void documentSaved();

private:
    explicit DocumentManager(QObject *parent = 0);
    virtual ~DocumentManager();

    class Private;
    Private * const d;

    static DocumentManager *sm_instance;

private Q_SLOTS:
    void delayedNewDocument();
    void delayedSaveAs();
    void delayedOpenDocument();
};

#endif // DOCUMENTMANAGER_H
