/*  This file is part of the KDE project

    Copyright (c) 2011 Sven Langkamp <sven.langkamp@gmail.com>
    Copyright (C) 2011 Srikanth Tiyyagura <srikanth.tulasiram@gmail.com>

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

#include "KoResourceTagStore.h"

#include <QDebug>
#include <QStringList>
#include <kstandarddirs.h>
#include <QFile>
#include <QDir>

KoResourceTagStore::KoResourceTagStore(const QString& resourceType, const QString& extensions)
{
    m_serverExtensions = extensions;
    m_tagsXMLFile =  KStandardDirs::locateLocal("data", "krita/tags/" + resourceType + "_tags.xml");
    m_config = KConfigGroup(KGlobal::config(), "resource tagging");
    readXMLFile();
}

KoResourceTagStore::~KoResourceTagStore()
{
    serializeTags();
}

QStringList KoResourceTagStore::assignedTagsList(KoResource* resource) const
{
    return m_tagRepo.values(adjustedFileName(resource->filename()));
}

QStringList KoResourceTagStore::tagNamesList() const
{
    return m_tagList.uniqueKeys();
}

void KoResourceTagStore::addTag(KoResource* resource, const QString& tag)
{
    if (!resource) {
        m_tagList.insert(tag, 0);
    }
    else {
        QString fileName = adjustedFileName(resource->filename());
        addTag(fileName, tag);
    }
}

void KoResourceTagStore::addTag(const QString& fileName, const QString& tag)
{
    if (m_tagRepo.contains(fileName, tag)) {
        return;
    }

    m_tagRepo.insert(fileName, tag);

    if (m_tagList.contains(tag)) {
        m_tagList[tag]++;
    } else {
        m_tagList.insert(tag, 1);
    }
}

void KoResourceTagStore::delTag(KoResource* resource, const QString& tag)
{
    QString fileName = adjustedFileName(resource->filename());

    if (!m_tagRepo.contains(fileName, tag)) {
        return;
    }

    m_tagRepo.remove(fileName, tag);

    if (m_tagList.contains(tag)) {
        if (m_tagList[tag] > 0) {
            m_tagList[tag]--;
        }
    }
}

QStringList KoResourceTagStore::searchTag(const QString& lineEditText)
{
    QStringList tagsList = lineEditText.split(QRegExp("[,]\\s*"), QString::SkipEmptyParts);
    if (tagsList.isEmpty()) {
        return QStringList();
    }

    QStringList keysList = m_tagRepo.keys(tagsList.takeFirst());

    if (tagsList.count() >= 1) {
        QStringList resultKeysList;
        bool tagPresence;
        foreach(const QString & key, keysList) {
            tagPresence = true;
            foreach(const QString & tag, tagsList) {
                if (!m_tagRepo.contains(key, tag)) {
                    tagPresence = false;
                    break;
                }
            }
            if (tagPresence) {
                resultKeysList.append(key);
            }
        }
        return removeAdjustedFileNames(resultKeysList);
    }
    return removeAdjustedFileNames(keysList);
}

void KoResourceTagStore::writeXMLFile(bool serverIdentity)
{
    Q_UNUSED(serverIdentity);
    QFile f(m_tagsXMLFile);
    //bool fileExists = f.exists();

    if (!f.open(QIODevice::WriteOnly | QIODevice::Text)) {
        kWarning() << "Cannot write meta information to '" << m_tagsXMLFile << "'.";
        return;
    }
    QDomDocument doc;
    QDomElement root;


//    if (!fileExists || !doc.setContent(&f)) {
//        createCleanFile = true;
//    }
//    else {
//        root = doc.documentElement();
//        if (root.tagName() != "tags") {
//            createCleanFile = true;
//        }
//    }

//    if (createCleanFile) {
        QDomDocument docTemp("tags");
        doc = docTemp;
        doc.appendChild(doc.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"UTF-8\""));
        root = doc.createElement("tags");
        doc.appendChild(root);
//    }

    QStringList resourceNames = m_tagRepo.uniqueKeys();

    resourceNames.replaceInStrings(QDir::homePath(), QString("~"));

//    if (fileExists) {
//        QDomNodeList resourceNodesList = root.childNodes();
//        /// resource are checked and added or removed according to need.
//        for (int i = 0; i < resourceNodesList.count() ; i++) {
//            QDomElement resourceEl = resourceNodesList.at(i).toElement();
//            if (resourceEl.tagName() == "resource") {
//                if (resourceNames.contains(resourceEl.attribute("identifier"))) {
//                    resourceNames.removeAll(resourceEl.attribute("identifier"));
//                    /// Tags are checked for a resource and added or removed according to need.
//                    QDomNodeList tagNodesList = resourceNodesList.at(i).childNodes();
//                    QStringList tags = m_tagRepo.values((resourceEl.attribute("identifier")).replace(QString("~"), QDir::homePath()));
//                    for (int j = 0; j < tagNodesList.count() ; j++) {
//                        QDomElement tagEl = tagNodesList.at(j).toElement();
//                        if (tags.contains(tagEl.text())) {
//                            tags.removeAll(tagEl.text());
//                        } else {
//                            resourceNodesList.at(i).removeChild(tagNodesList.at(j--));
//                        }
//                    }
//                    foreach(const QString & tag, tags) {
//                        QDomElement newTagEl = doc.createElement("tag");
//                        QDomText tagNameText = doc.createTextNode(tag);
//                        newTagEl.appendChild(tagNameText);
//                        resourceNodesList.at(i).appendChild(newTagEl);
//                    }
//                } else {
//                    if (isServerResource((resourceEl.attribute("identifier")).replace(QString("~"), QDir::homePath())) || !serverIdentity) {
//                        root.removeChild(resourceNodesList.at(i--));
//                    }
//                }
//            }
//        }
//    }

    foreach(QString resourceName, resourceNames) {

        QDomElement resourceEl = doc.createElement("resource");
        resourceEl.setAttribute("identifier", resourceName);

        QStringList tags = m_tagRepo.values(resourceName.replace(QString("~"), QDir::homePath()));
        foreach(const QString & tag, tags) {
            QDomElement tagEl = doc.createElement("tag");
            QDomText tagNameText = doc.createTextNode(tag);
            tagEl.appendChild(tagNameText);
            resourceEl.appendChild(tagEl);
        }

        root.appendChild(resourceEl);
    }

    // Now write empty tags
    foreach(const QString &tag, m_tagList.uniqueKeys())  {
        if (m_tagList[tag] == 0) {
            QDomElement resourceEl = doc.createElement("resource");
            resourceEl.setAttribute("identifier", "dummy");
            QDomElement tagEl = doc.createElement("tag");
            QDomText tagNameText = doc.createTextNode(tag);
            tagEl.appendChild(tagNameText);
            resourceEl.appendChild(tagEl);
            root.appendChild(resourceEl);
        }
    }

//    f.remove();
//    if (!f.open(QIODevice::WriteOnly | QIODevice::Text)) {
//        kWarning() << "Cannot write meta information to '" << m_tagsXMLFile << "'.";
//    }
    QTextStream metastream(&f);
    metastream << doc.toByteArray();

    f.close();

}

void KoResourceTagStore::readXMLFile(bool serverIdentity)
{

    QString inputFile;

    if (QFile::exists(m_tagsXMLFile)) {
        inputFile = m_tagsXMLFile;
    } else {
        inputFile = KStandardDirs::locateLocal("data", "krita/tags.xml");
    }

    QFile f(inputFile);
    if (!f.open(QIODevice::ReadOnly)) {
        return;
    }

    QDomDocument doc;
    if (!doc.setContent(&f)) {
        kWarning() << "The file could not be parsed.";
        return;
    }

    QDomElement root = doc.documentElement();
    if (root.tagName() != "tags") {
        kWarning() << "The file doesn't seem to be of interest.";
        return;
    }

    QDomNodeList resourceNodesList = root.childNodes();

    for (int i = 0; i < resourceNodesList.count(); i++) {
        QDomElement resourceEl = resourceNodesList.at(i).toElement();
        if (resourceEl.tagName() == "resource") {
            QString resourceName = resourceEl.attribute("identifier");
            resourceName.replace(QString("~"), QDir::homePath());
            if (resourceName == "dummy" || isServerResource(resourceName) || !serverIdentity) {
                QDomNodeList tagNodesList = resourceNodesList.at(i).childNodes();
                for (int j = 0; j < tagNodesList.count() ; j++) {
                    QDomElement tagEl = tagNodesList.at(j).toElement();
                    addTag(resourceName, tagEl.text());
                }
            }
        }
    }


}

bool KoResourceTagStore::isServerResource(const QString &resourceName) const
{
    bool removeChild = false;
    QStringList extensionsList = m_serverExtensions.split(':');
    foreach(QString extension, extensionsList) {
        if (resourceName.contains(extension.remove('*'))) {
            removeChild = true;
            break;
        }
    }
    return removeChild;
}

QString KoResourceTagStore::adjustedFileName(const QString &fileName) const
{
    if (!isServerResource(fileName)) {
        return fileName + "-krita" + m_serverExtensions.split(':').takeFirst().remove('*');
    }
    return fileName;
}

QStringList KoResourceTagStore::removeAdjustedFileNames(QStringList fileNamesList)
{
    foreach(const QString & fileName, fileNamesList) {
        if (fileName.contains("-krita")) {
            fileNamesList.append(fileName.split("-krita").takeFirst());
            fileNamesList.removeAll(fileName);
        }
    }
    return fileNamesList;
}



void KoResourceTagStore::serializeTags()
{
    writeXMLFile();
}
