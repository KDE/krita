/*
 *    This file is part of the KDE project
 *    Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
 *    Copyright (c) 2007 Jan Hambrecht <jaham@gmx.net>
 *    Copyright (c) 2007 Sven Langkamp <sven.langkamp@gmail.com>
 *    Copyright (C) 2011 Srikanth Tiyyagura <srikanth.tulasiram@gmail.com>
 *    Copyright (c) 2011 José Luis Vergara <pentalis@gmail.com>
 *    Copyright (c) 2013 Sascha Suelzer <s.suelzer@gmail.com>
 *    Copyright (c) 2019 Boudewijn Rempt <boud@valdyas.org>
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Library General Public
 *    License as published by the Free Software Foundation; either
 *    version 2 of the License, or (at your option) any later version.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Library General Public License for more details.
 *
 *    You should have received a copy of the GNU Library General Public License
 *    along with this library; see the file COPYING.LIB.  If not, write to
 *    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *    Boston, MA 02110-1301, USA.
 */

#ifndef KISRESOURCETAGGINGMANAGER_H
#define KISRESOURCETAGGINGMANAGER_H

#include <QObject>

#include <KoResource.h>

class QWidget;
class QStringList;
class QString;
class QPoint;

class KisTagFilterWidget;
class KisTagChooserWidget;
class KisTagFilterResourceProxyModel;

/**
 * @brief The KisResourceTaggingManager class is ...
 *
 * XXX: this needs to be documented!
 */
class KisResourceTaggingManager : public QObject
{
    Q_OBJECT

public:

    explicit KisResourceTaggingManager(KisTagFilterResourceProxyModel *model, QWidget *parent);
    ~KisResourceTaggingManager() override;
    void showTaggingBar(bool show);
    QStringList availableTags() const;
    void contextMenuRequested(KoResourceSP currentResource, QPoint pos);
    void allowTagModification( bool set );
    bool allowTagModification();
    KisTagFilterWidget *tagFilterWidget();
    KisTagChooserWidget *tagChooserWidget();

Q_SIGNALS:

    void updateView();
    
private Q_SLOTS:

    void undeleteTag(const QString& tagToUndelete);
    void purgeTagUndeleteList();
    void contextCreateNewTag(KoResourceSP resource, const QString& tag);
    void contextCreateNewTag(const QString& tag);
    void syncTagBoxEntryRemoval(const QString& tag);
    void syncTagBoxEntryAddition(const QString& tag);
    void syncTagBoxEntries();
    void tagSaveButtonPressed();
    void contextRemoveTagFromResource(KoResourceSP resource, const QString& tag);
    void contextAddTagToResource(KoResourceSP resource, const QString& tag);
    void renameTag(const QString &oldName, const QString &newName);
    void tagChooserIndexChanged(const QString& lineEditText);
    void tagSearchLineEditTextChanged(const QString& lineEditText);
    void removeTagFromComboBox(const QString& tag);

private:

    void contextMenuRequested(KoResourceSP resource, const QStringList& resourceTags, const QPoint& pos);
    void enableContextMenu(bool enable);
    void removeResourceTag(KoResourceSP resource, const QString& tagName);
    void addResourceTag(KoResourceSP resource, const QString& tagName);
    void updateTaggedResourceView();

    class Private;
    Private* const d;
};


#endif // KORESOURCETAGGINGINTERFACE_H
