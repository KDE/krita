/*  This file is part of the KDE project

    Copyright (c) 2011 Sven Langkamp <sven.langkamp@gmail.com>
    Copyright (c) 2011 Srikanth Tiyyagura <srikanth.tulasiram@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef KORESOURCETAGSTORE_H
#define KORESOURCETAGSTORE_H


#include <kdebug.h>
#include <KoResource.h>
#include <QDomDocument>
#include "kowidgets_export.h"
#include <kconfiggroup.h>
#include <KoConfig.h>

class KoResourceServerBase;

/**
 * KoResourceTagging allows to add and delete tags to resources and also search reources using tags
 */
class KOWIDGETS_EXPORT KoResourceTagStore
{
public:

    /**
    * Constructs a KoResourceTagging object
    *
    */
    explicit KoResourceTagStore(KoResourceServerBase *resourceServer, const QString& resourceType, const QString& extensions);
    ~KoResourceTagStore();

    QStringList assignedTagsList(KoResource* resource) const;

    /// remote the given resource from the tagstore
    void removeResource(const KoResource *resource);

    /// Add the given tag to the tag store. The resource can be empty, in which case
    /// the tag is added but unused
    void addTag(KoResource* resource, const QString& tag);

    /// Remove the given tag for the given resource.
    void delTag(KoResource* resource, const QString& tag);

    void delTag(const QString& tag);

    /// @return a list of all the tags in this store
    QStringList tagNamesList() const;

    /// Return a list of filenames for the given tag
    QStringList searchTag(const QString& query) const;

    void loadTags();
    void serializeTags();

private:
    friend class KoResourceTaggingTest;

    void readXMLFile(const QString &tagstore);
    void writeXMLFile(const QString &tagstore);

    /// To check whether the resource belongs to the present server or not
    bool isServerResource(const QString &resourceName) const;

    /// If resource filenames have no extensions, then we add "-krita.extension".
    QString adjustedFileName(const QString &fileName) const;

    /// Removes the adjustements before going to the server
    QStringList removeAdjustedFileNames(QStringList fileNamesList) const;

    class Private;
    Private * const d;
};


#endif // KORESOURCETAGSTORE_H
