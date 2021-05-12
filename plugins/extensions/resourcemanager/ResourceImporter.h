/*
 * SPDX-FileCopyrightText: 2021 Agata Cacko <cacko.azh@gmail.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef RESOURCE_IMPORTER_H
#define RESOURCE_IMPORTER_H

#include <KoDialog.h>
#include <QScopedPointer>
#include <QSortFilterProxyModel>
#include <QSharedPointer>
#include <QItemSelection>
#include <KisResourceThumbnailPainter.h>

class KisResourceModel;
class KisMainWindow;

class ResourceImporter : public QObject
{
    Q_OBJECT
public:
    ResourceImporter(QWidget* parent);

    ~ResourceImporter() override;

public:
    void importResources(QString startPath = "");



private Q_SLOTS:

private:
    void prepareTypesMaps();
    void prepareModelsMap();
    void initialize();


    enum ImportFailureReason
    {
        MimetypeResourceTypeUnknown, // when someone tries to import a .foo file
        ResourceCannotBeLoaded,
        CancelledByTheUser, // the user pressed Cancel when asked
        StorageAlreadyExists, // bundle, asl or abr file already exists in that location
    };

private:

    friend class FailureReasonsDialog;

    QMap<QString, KisResourceModel*> m_resourceModelsForResourceType;
    QMap<QString, QStringList> m_resourceTypesForMimetype;
    QMap<QString, QStringList> m_mimetypeForResourceType;
    QStringList m_storagesMimetypes;
    QStringList m_zipMimetypes;
    QStringList m_allMimetypes;
    QWidget* m_widgetParent {0};

    bool m_isInitialized {false};

};

#endif // RESOURCE_IMPORTER_H
