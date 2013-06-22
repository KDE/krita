/*  This file is part of the KDE project

    Copyright (c) 2011 Sven Langkamp <sven.langkamp@gmail.com>
    Copyright (c) 2011 Srikanth Tiyyagura <srikanth.tulasiram@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef KORESOURCETAGGING_H
#define KORESOURCETAGGING_H


#include <kdebug.h>
#include <KoResource.h>
#include <QDomDocument>
#include "kowidgets_export.h"
#include <kconfiggroup.h>
#include <ksharedconfig.h>
#include <KoConfig.h>


#ifdef NEPOMUK
#include <Nepomuk/Resource>
#endif
/**
 * KoResourceTagging allows to add and delete tags to resources and also search reources using tags
 */
class KOWIDGETS_EXPORT KoResourceTagging {

public:

    /**
    * Constructs a KoResourceTagging object
    *
    */
    explicit KoResourceTagging(const QString& resourceType, const QString& extensions);
    ~KoResourceTagging();

    QStringList assignedTagsList(KoResource* resource) const;

    void addTag(KoResource* resource,const QString& tag);

    void delTag(KoResource* resource,const QString& tag);

    QStringList tagNamesList() const;

    QStringList searchTag(const QString& tag);

    void serializeTags();


    void setNepomukBool(bool nepomukOn);
#ifdef NEPOMUK
    void updateNepomukXML(bool nepomukOn);
#endif

private:
    void readXMLFile(const QString& filename, bool serverIdentity=true);
    void writeXMLFile(bool serverIdentity=true);

    /// To check whether the resource belongs to the present server or not
    bool isServerResource(const QString &resourceName) const;
    void addTag(const QString& fileName,const QString& tag);
    /// If resource filenames have no extensions, then we add "-krita.extension".
    QString adjustedFileName(const QString &fileName) const;
    /// Removes the adjustements before going to the server
    QStringList removeAdjustedFileNames(QStringList fileNamesList);

#ifdef NEPOMUK
    QList<Nepomuk::Resource> readNepomukRepo();
    void writeNepomukRepo(bool serverIdentity=true);
    void updateTagRepoFromNepomuk(bool serverIdentity=true);
    void addNepomukTag(const QString& fileName,const QString& tag);
    void delNepomukTag(const QString& fileName,const QString& tag);
    QString adjustedNepomukFileName(QString fileName) const;
    QString correctedNepomukFileName(QString fileName) const;
#endif

    QMultiHash<QString, QString> m_tagRepo;
    QHash<QString, int> m_tagList;
    QString m_tagsXMLFile;
    QString m_serverExtensions;

    bool m_nepomukOn;
    KConfigGroup m_config;
};


#endif // KORESOURCETAGGING_H
