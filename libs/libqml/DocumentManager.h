/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2012 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef DOCUMENTMANAGER_H
#define DOCUMENTMANAGER_H

#include <QObject>
#include <QVariantMap>

#include "krita_sketch_export.h"

class RecentFileManager;
class Settings;
class ProgressProxy;
class KisDocument;

class KRITA_SKETCH_EXPORT DocumentManager : public QObject
{
    Q_OBJECT
public:
    KisDocument* document() const;
    ProgressProxy* progressProxy() const;
    Settings* settingsManager() const;
    void setSettingsManager(Settings* newManager);
    RecentFileManager* recentFileManager() const;
    bool isTemporaryFile() const;

public Q_SLOTS:
    void newDocument(int width, int height, float resolution);
    void newDocument(const QVariantMap& options);
    void openDocument(const QString& document, bool import = false);
    void closeDocument();
    bool save();
    void saveAs(const QString &filename, const QString &mimetype);
    void reload();
    void setTemporaryFile(bool temp);

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

    void onLoadCompleted();
    void onLoadCanceled(const QString &errMsg);
};

#endif // DOCUMENTMANAGER_H
