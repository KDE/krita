/*
 *    This file is part of the KDE project
 *    Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
 *    Copyright (c) 2007 Jan Hambrecht <jaham@gmx.net>
 *    Copyright (c) 2007 Sven Langkamp <sven.langkamp@gmail.com>
 *    Copyright (C) 2011 Srikanth Tiyyagura <srikanth.tulasiram@gmail.com>
 *    Copyright (c) 2011 José Luis Vergara <pentalis@gmail.com>
 *    Copyright (c) 2013 Sascha Suelzer <s.suelzer@gmail.com>
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

#ifndef KORESOURCETAGGINGMANAGER_H
#define KORESOURCETAGGINGMANAGER_H

#include <QObject>

class QWidget;
class QStringList;
class QString;
class QPoint;

class KoTagFilterWidget;
class KoTagChooserWidget;
class KoResourceModel;
class KoResource;

/**
 * @brief The KoResourceTaggingManager class is ...
 *
 * XXX: this needs to be documented!
 */
class KoResourceTaggingManager : public QObject
{
    Q_OBJECT

public:

    explicit KoResourceTaggingManager(KoResourceModel* model, QWidget* parent);
    ~KoResourceTaggingManager() override;
    void showTaggingBar(bool show);
    QStringList availableTags() const;
    void contextMenuRequested(KoResource* currentResource, QPoint pos);
    void allowTagModification( bool set );
    bool allowTagModification();
    KoTagFilterWidget* tagFilterWidget();
    KoTagChooserWidget* tagChooserWidget();

Q_SIGNALS:
    void updateView();
    
private Q_SLOTS:

    void undeleteTag(const QString& tagToUndelete);
    void purgeTagUndeleteList();
    void contextCreateNewTag(KoResource* resource, const QString& tag);
    void contextCreateNewTag(const QString& tag);
    void syncTagBoxEntryRemoval(const QString& tag);
    void syncTagBoxEntryAddition(const QString& tag);
    void syncTagBoxEntries();
    void tagSaveButtonPressed();
    void contextRemoveTagFromResource(KoResource* resource, const QString& tag);
    void contextAddTagToResource(KoResource* resource, const QString& tag);
    void renameTag(const QString &oldName, const QString &newName);
    void tagChooserIndexChanged(const QString& lineEditText);
    void tagSearchLineEditTextChanged(const QString& lineEditText);
    void removeTagFromComboBox(const QString& tag);

private:
    void contextMenuRequested(KoResource* resource, const QStringList& resourceTags, const QPoint& pos);
    void enableContextMenu(bool enable);
    void removeResourceTag(KoResource* resource, const QString& tagName);
    void addResourceTag(KoResource* resource, const QString& tagName);
    void updateTaggedResourceView();
    class Private;
    Private* const d;
};


#endif // KORESOURCETAGGINGINTERFACE_H
