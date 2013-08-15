/*
 *    This file is part of the KDE project
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

#ifndef KORESOURCETAGGINGINTERFACE_H
#define KORESOURCETAGGINGINTERFACE_H

#include "KoTagFilterWidget.h"
#include "KoTagChooserWidget.h"

class KoResourceModel;
class KoResource;

class KoResourceTaggingInterface : public QObject
{
    Q_OBJECT

public:
    explicit KoResourceTaggingInterface(KoResourceModel*, QWidget* parent);
    void showTaggingBar(bool showSearchBar, bool showOpBar);
    QStringList availableTags() const;
    QString currentTag();
    void contextMenuRequested(KoResource* currentResource, QPoint pos);
    KoTagFilterWidget* tagFilterWidget();
    KoTagChooserWidget* tagChooserWidget();
private slots:
    void undeleteTag(const QString& tagToUndelete);
    void purgeTagUndeleteList();
    void contextCreateNewTag(KoResource* resource, const QString& tag);
    void contextCreateNewTag(const QString& tag);
    void syncTagBoxEntryRemoval(const QString& tag);
    void syncTagBoxEntryAddition(const QString& tag);
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
